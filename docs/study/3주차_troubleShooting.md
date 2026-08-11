# 3주차 트러블슈팅

> 조명 시스템(`SpaceLightingSystem`/`Glow`)과 "지구가 실제로 공전하기 시작하면서" 터진
> 궤도역학 버그들을 증상 → 원인 → 해결 순으로 남긴다.
> 기술 회고 문서 쓸 때 재료로 그대로 쓸 수 있게, 코드 위치(파일/함수)를 같이 적어둔다.

---

# Part 1. 조명 시스템(`SpaceLightingSystem`/`Glow`) 첫 구현 버그

## 1. `Glow::Render()`가 항상 아무것도 안 그림 (조건 반전)

**증상**
태양 글로우, 엔진 파티클, 블랙홀 강착원반 글로우 — `Glow`를 쓰는 곳 전부 화면에 안 보임.

**원인**
```cpp
// Glow.cpp
void Glow::Render(HDC hdcDest, const Vector2& centerPos, float radius, BYTE intensity)
{
    if (_hdc) return;   // ← 부호가 반대
    ...
}
```
`Create()`가 성공하면 `_hdc`는 유효한(non-null) 값이 되므로, 정상적으로 생성된 `Glow`는
**항상 이 줄에서 리턴**해버린다. "아직 `Create()` 안 된 `Glow`는 그리지 마라"는 방어 코드를
넣으려다 조건 부호를 반대로 씀.

**해결**
```cpp
if (!_hdc) return;   // 생성 안 됐으면 그리지 않는다
```

---

## 2. `ApplyPlanetShadow()`의 그림자가 엉뚱한 모양으로 나옴 (점-직선 거리 공식 오류)

**증상**
행성 그림자가 거의 안 보이거나, 보여도 태양 근처에서만 이상하게 나타남.

**원인**
```cpp
Vector2 dir = sunToPlanet / distSunPlanet;      // 태양→행성 축의 단위벡터
float proj = sunToPixel.Dot(dir);               // 그 축 위로의 투영 거리
...
float perpDist = sqrt(max(0.f, sunToPixel.LengthSquared()));  // ← proj를 안 씀
```
`perpDist`(그림자 축에서 벗어난 정도)를 구하려면 피타고라스 정리로
`|sunToPixel|² = proj² + perpDist²` → `perpDist = sqrt(|sunToPixel|² - proj²)`가 맞는데,
`- proj * proj`가 빠져서 `perpDist`가 그냥 `sunToPixel.Length()`(태양~픽셀 직선거리)가
되어버렸다. 결과적으로 "태양 축보다 뒤쪽에 있으면서 **태양 자체로부터** `planetRadius`
이내"인 픽셀을 찾는 꼴이 되어, 행성이 태양에서 멀리 있으면 조건을 만족하는 픽셀이
거의 없다.

**해결**
```cpp
float perpDist = sqrt(max(0.f, sunToPixel.LengthSquared() - proj * proj));
```

---

## 3. `SpaceLightingSystem::Release()`가 `Glow` 3개를 delete 안 함 (메모리 누수)

**증상 (코드 리뷰 중 발견)**
`Initialize()`에서 `_sunGlow`/`_engineGlow`/`_accretionGlow` 세 개를 `new`로 생성하는데,
`Release()`(소멸자에서도 호출됨)엔 대응하는 `delete`가 없음.

**원인**
`Release()`가 `_pBgPixels`, `_hDIBDC`/`_hDIBBitmap` 정리만 하고 `Glow` 포인터 세 개는
빠뜨림.

**해결**
```cpp
delete _sunGlow;       _sunGlow = nullptr;
delete _engineGlow;    _engineGlow = nullptr;
delete _accretionGlow; _accretionGlow = nullptr;
```
> 📌 같이 발견한 사소한 것: `_engineGlow`만 헤더에서 `= nullptr` 기본값이 없었음
> (`_sunGlow`/`_accretionGlow`는 있었음) — 통일해서 같이 고침.

---

## 4. `BeginRender()`에서 즉시 크래시 (`memset`을 `memcpy`로 오타)

**증상**
```cpp
memcpy(_pBgPixels, 0,  _width * _height * 4);
```
이 줄에서 바로 접근 위반(access violation).

**원인**
버퍼를 0으로 초기화하려던 의도였는데 `memset`을 써야 할 자리에 `memcpy`를 씀.
`memcpy(dest, src, size)`의 `src` 자리에 `0`(=`NULL`)이 들어가서 "주소 0x0부터
읽어서 복사하라"가 되어버려 그 즉시 크래시.

**해결**
```cpp
memset(_pBgPixels, 0, _width * _height * 4);
```
> 📌 `memset(dest, value, size)`와 `memcpy(dest, src, size)`는 인자 개수와 순서가
> 비슷해서 헷갈리기 쉽지만, 2번째 인자의 의미(채울 값 vs 복사할 원본 주소)가
> 완전히 다르다.

---

# Part 2. 지구를 실제로 공전시키면서 터진 물리 버그

배경: `earth->Setup(..., orbitSpeed=0.02f, ...)`로 지구를 실제로 돌리기 시작하면서,
"우주선도 지구를 따라 움직여야 한다"는 요구사항이 생겼고 그 과정에서 아래 버그들이 연쇄로 드러남.

## 5. `APlanet::GetVelocity()`가 지구 속도를 36억으로 계산 (제곱 오타)

**증상**
지구 공전속도를 우주선 초기속도에 더해줬더니, 우주선이 스폰되자마자 화면에서
완전히 사라짐(카메라가 우주선을 따라가다 저 멀리 날아가버림).

**원인**
```cpp
Vector2 APlanet::GetVelocity() const
{
    return Vector2(-sinf(_angle), cosf(_angle)) * (_orbitRadius * _orbitRadius);  // ← 제곱
}
```
원운동 접선속도 공식은 `v = r·ω`(반지름 × **각속도**)인데 `_orbitRadius`를 두 번
곱해버림(제곱). 지구는 `orbitRadius=60000, orbitSpeed=0.02`라 의도한 값은
`60000*0.02=1200`인데 실제로는 `60000*60000=36억`이 나옴.

**해결**
```cpp
return Vector2(-sinf(_angle), cosf(_angle)) * (_orbitRadius * _orbitSpeed);
```

---

## 6. 궤도 판정(`OrbitalGameMode`)이 지구가 조금만 움직여도 바로 "탈출"로 오판

**증상**
지구가 공전을 시작하자, 정상적으로 원궤도에 있어도 HUD가 바로 `Failed`(탈출/추락)로
판정됨.

**원인**
```cpp
Vector2 r = _ship->GetCenterPos() - home->GetCenterPos();          // 상대 위치 (맞음)
Vector2 v = _ship->GetComponent<UPhysicsComponent>()->GetVelocity();  // 절대 속도 (틀림)
_eps = 0.5f * v.LengthSquared() - mu / r.Length();
```
궤도 에너지/각운동량 공식은 위치·속도가 **둘 다** 중심 천체(지구) 기준 상대좌표일 때만
성립한다. `r`은 상대 위치로 맞게 계산했는데 `v`는 절대(월드) 속도를 그대로 써서,
지구 자체 속도(~1200)가 그대로 섞여 `_eps`가 항상 크게 양수로 나와버림.

**해결**
```cpp
Vector2 v = _ship->GetComponent<UPhysicsComponent>()->GetVelocity() - home->GetVelocity();
```

---

## 7. `KEY_0`(디버그 궤도 진입)을 다시 눌러도 판정 상태가 리셋 안 됨

**증상**
한 번 `Failed`가 뜬 뒤 `KEY_0`으로 새 원궤도를 넣어줘도, HUD가 계속 `Failed`로 고정됨.

**원인**
```cpp
if (_orbitState == EOrbitState::Flying && _graceTimer >= kGracePeriod)   // Flying일 때만 판정
{
    if (crashedNow || _isEscaping) _orbitState = EOrbitState::Failed;
    ...
}
```
판정 전이는 `_orbitState == Flying`일 때만 일어나므로, 한 번 `Failed`가 되면 그 이후로는
**영원히 재판정되지 않는다.** `KEY_0`이 우주선의 위치/속도는 리셋해도 `OrbitalGameMode`의
상태는 안 건드림.

**해결**
```cpp
// OrbitalGameMode.h
void ResetJudgment() { _orbitState = EOrbitState::Flying; _graceTimer = 0.f; _isWarning = false; }

// MainWorld.cpp, KEY_0 블록 마지막에 추가
GetGameMode<OrbitalGameMode>()->ResetJudgment();
```

---

## 8. 우주선이 궤도에 안착해도 지구를 놓치고 이탈 (조석력 보정 없음)

**증상**
위 6~7번을 고쳐도, 우주선이 지구 궤도에 진입한 뒤 시간이 지나면 지구를 놓치고
계속 멀어짐 — "따라가기만도 벅차다"는 증상.

**원인 (숫자로 확인)**
- 지구가 자기 공전궤도를 유지하려고 받는 구심가속도: `a_earth = ω²·r = 0.02²×60000 = 24`
- 우주선이 지구 궤도(고도 300)에서 받는 지구 중력가속도: `a_local = μ/r² ≈ 11.8`

`a_earth`가 `a_local`보다 **두 배나 커서**, 지구를 계속 밀어붙이는 가속도를 우주선이
못 버티고 튕겨 나간다. 현실에서는 태양이 지구도 당기고 지구 근처의 우주선도 거의
똑같이 당겨서 이 효과가 상쇄되는데, 이 프로젝트는 태양 중력을 아예 시뮬레이션 안 하므로
그 상쇄가 빠져있었음.

**해결**
지구가 받는 것과 같은 가속도를 우주선에도 보정으로 더해줘서, "태양이 둘 다 당기는" 효과를
흉내낸다.
```cpp
// APlanet.h/.cpp
Vector2 GetAcceleration() const
{
    return (_orbitCenter - GetCenterPos()) * (_orbitSpeed * _orbitSpeed);
}

// UPhysicsComponent::ComputeAcceleration()
a += dir * (targetPlanet->GetMu() / (r2 * r));   // 기존 중력항
a += targetPlanet->GetAcceleration();            // ★ 추가: 조석력 상쇄
```
> 📌 **수학적으로 확인한 것:** 상대좌표 `ρ = ship.pos - home.pos`로 놓고 보면, 이 보정을
> 더했을 때 `d²ρ/dt² = g_earth(ship)`로 **지구의 움직임이 수식에서 완전히 사라진다.**
> 즉 지구가 아무리 빨리 돌아도, 상대궤도(우주선 입장에서 본 궤도 모양)는 지구가
> 정지해 있을 때와 수학적으로 동일해진다.

---

# Part 3. 성능 문제가 물리 버그로 이어진 사례

## 9. `ApplyPlanetShadow()`가 화면 전체(60만 픽셀)를 매 프레임 스캔

**증상 (코드 리뷰 중 발견, 이후 10번의 원인이 됨)**
```cpp
for (int y = 0; y < _height; ++y)      // 0~800 전체
    for (int x = 0; x < _width; ++x)   // 0~600 전체
```
바로 아래 `ApplyBlackHoleLensing()`은 영향 반경만 스캔하게 바운딩 박스가 있는데,
`ApplyPlanetShadow()`만 이게 빠져서 매 프레임 600×800 픽셀을 전부 훑고 있었음.
Debug 빌드(최적화 없음)에서 이 정도 픽셀 루프는 프레임당 수십 ms까지 걸릴 수 있음.

**해결**
그림자가 뻗어나가는 먼 쪽 끝점(화면 대각선 길이만큼)을 계산해서, 그 주변만 스캔하도록
바운딩 박스를 씌움.
```cpp
float maxShadowLen = sqrtf((float)(_width * _width + _height * _height));
Vector2 farPoint = planetPos + dir * maxShadowLen;
int minX = (int)max(0.f, min(planetPos.x, farPoint.x) - planetRadius);
int maxX = (int)min((float)_width,  max(planetPos.x, farPoint.x) + planetRadius);
// minY, maxY도 동일하게
for (int y = minY; y < maxY; ++y)
    for (int x = minX; x < maxX; ++x) { ... }
```

---

## 10. 위 9번 고치면서 `x < maxY` 오타 (화면이 지저분하게 깨짐)

**증상**
바운딩 박스를 넣은 뒤 행성 주변이 지저분하게 깨져 보이고, 우주선 궤도까지 이상해
보임.

**원인**
```cpp
for (int y = minY; y < maxY; ++y)
    for (int x = minX; x < maxY; ++x)   // ← maxX가 아니라 maxY
```
`_width=600 < _height=800`이라 `maxY`가 `maxX`보다 클 때가 많은데, 그러면 `x`가 600을
훌쩍 넘어서(최대 800 근처까지) 돌아버림. `idx = (y * _width + x) * 4`가 원래 그 줄(row)을
벗어나 **다음 줄들의 픽셀 데이터까지 침범**하고, 화면 아래쪽 근처에서는 배열 범위 자체를
벗어나는 힙 오버런까지 날 수 있음.

**해결**
```cpp
for (int x = minX; x < maxX; ++x)
```

---

## 11. ⭐ `MAX_STEPS`가 `GMaxDeltaTime`을 다 못 따라가서 "지구-우주선 시간"이 어긋남 (근본 원인)

**증상**
9~10번을 다 고쳐도 여전히, 시간이 지나면 우주선이 지구를 놓치고 이탈. Release 빌드로
테스트하면 정상(궤도가 늘어난 타원이 됐다가 다시 돌아옴)인데 Debug 빌드에서만 이탈.

**원인**
```cpp
// pch.h
constexpr float GMaxDeltaTime = 1.0f / 10.0f;      // deltaTime은 0.1초까지 허용(클램프)

// UPhysicsComponent.h
static constexpr float FIXED_DT = 1.f / 240.f;
static constexpr int MAX_STEPS = 5;                // 최대 5×(1/240) ≈ 0.0208초까지만 소비
```
`TimeManager`에 이미 deltaTime 클램프(`GMaxDeltaTime=0.1초`)가 있었지만, 우주선 물리가
한 프레임에 실제로 처리할 수 있는 최대 시간(`MAX_STEPS × FIXED_DT ≈ 0.0208초`)이 그보다
**5배나 작았다.** 프레임이 0.0208초보다만 느려져도(48fps 밑) — Debug 빌드에서는 흔함,
특히 9번의 그림자 전체화면 스캔이 겹치면 더 심함 — 지구는 클램프된 deltaTime 전체를
다 돌아가는데(`APlanet::Update()`는 캡이 없음) 우주선 물리는 5스텝만 처리하고 나머지
시간은 그냥 버려짐(`if (steps >= MAX_STEPS) _accumulator = 0.f;`). 이게 매 프레임 쌓이면서
지구가 우주선 시뮬레이션보다 계속 앞서나가, 8번에서 어렵게 유도한 조석력 상쇄가
"같은 시간만큼 전진"이라는 전제가 깨져버림.

**해결**
```cpp
static constexpr int MAX_STEPS = 25;   // 5 → 25 (0.1초 / (1/240) ≈ 24, 여유 좀 두고 25)
```
프레임이 아무리 느려져도(`GMaxDeltaTime`으로 0.1초까지만 클램프되므로) 우주선 물리가
그 시간을 전부 따라잡을 수 있어, 지구와 우주선이 항상 같은 시간만큼 전진하게 됨.
평소(빠른 프레임)엔 여전히 스텝 1~4개만 돌아서 성능에 영향 없음.

> 📌 **검증 방법:** 게임 코드와 동일한 물리 공식을 파이썬으로 그대로 재현해서
> 시뮬레이션 → 이론적으로는 안정적(상대거리 1298~1300 유지)인데 실제 게임(Debug
> 빌드)에서는 이탈 → Release 빌드로 재현했더니 정상 → "프레임 속도가 원인"이라는
> 가설을 실제 빌드 비교로 확정. 코드만 읽어서는 못 잡을 뻔한, 프레임타임 의존적인
> 버그였음.

---

# Part 4. 예상 궤도(`PredictPath`) 설계 문제

## 12. 예측 경로가 화면에 거의 안 보임 (지구의 "미래" 위치를 써서 순식간에 화면 밖으로)

**증상**
예상 궤도(청록색 점 200개)가 화면에 거의 안 찍힘 — ImGui로 좌표를 직접 찍어보니
`predicted.size()`는 200으로 정상인데, 실제 좌표는:
```
ship: (44274, 41817)
p[0]  (0.5초 뒤): (43855, 42306)   → 화면 근처 (이거 하나만 보였음)
p[50] (25초 뒤): (16135, 58465)   → 3만 유닛 넘게 벗어남
p[199](100초 뒤): (-57260, 21654) → 완전히 딴 곳
```

**원인**
```cpp
path.push_back(relPos + home->GetFuturePos(t));   // 지구의 "그 시점" 실제 미래 위치를 더함
```
수학적으로는 정확했지만, 지구가 초당 1200으로 움직이다 보니 25초만 지나도 지구 자체가
3만 유닛 넘게 이동해버려서, 예측 경로가 "우주선이 지구 주위를 도는 모양"이 아니라
"지구를 따라 태양 주위를 도는 궤적 전체"가 되어버림. 카메라는 우주선 근처(로컬 영역)만
비추므로 첫 몇 개 점 빼고는 전부 시야 밖으로 벗어남.

**해결**
플레이어가 보고 싶은 건 "궤도의 모양"이지 지구의 미래 절대 위치가 아니므로, 지구의
**현재** 화면 위치에 상대궤도 모양을 겹쳐서 그림.
```cpp
Vector2 homePos = home->GetCenterPos();   // GetFuturePos(t) 대신 현재 위치, 루프 밖에서 한 번만
for (...) { ... path.push_back(relPos + homePos); }
```
> 📌 8번의 조석력 상쇄 덕분에 상대궤도(`relPos`)의 "모양" 자체는 지구의 실제 이동과
> 무관하게 정확하다 — 화면에 그릴 때 어디에 "고정"시켜서 보여줄지만 설계 문제였다.

---

# Part 5. (참고) 관찰 중 — 완전히 해결됐다고 단정하기엔 이름

## 13. ⚠️ 원궤도 진입 직후엔 완벽한데, 시간이 지나면 근지점/원지점이 미세하게 흔들림

**증상**
`KEY_0` 직후: 근지점 1299.8 / 원지점 1301.6 (거의 완벽한 원).
30초 뒤: 근지점 1275.8 / 원지점 1307.4 (약 ±2.4% 흔들림, `OrbitState`는 `Cleared` 유지).

**원인 (추정, 미확정)**
`APlanet::Update()`(지구 위치 갱신)는 프레임당 한 번만 실행되는데, 우주선의 물리
서브스텝(`UPhysicsComponent`)은 한 프레임 안에서 여러 번(많으면 최대 25번) 돌 수 있다.
그 서브스텝들이 전부 **그 프레임 시작 시점에 고정된 지구 위치/가속도**를 참조하므로,
8번에서 유도한 "조석력 상쇄가 정확히 성립하려면 지구와 우주선이 같은 순간에 같이
갱신돼야 한다"는 전제가 프레임 단위로는 살짝 어긋난다. 11번(`MAX_STEPS`)만큼 크지는
않지만 완전히 0은 아닌 잔차로 추정.

**해결 (필요해지면)**
지금 수준(±2.4%)은 게임플레이에 거의 안 느껴질 정도라 일단 보류. 더 정밀하게 만들려면
`ComputeAcceleration()` 안에서 우주선 서브스텝이 몇 번째인지에 따라 `APlanet::GetFuturePos()`
같은 방식으로 지구 위치를 그 순간에 맞게 미세 보간해줘야 함 — 서브스텝 단위 동기화라
지금보다 구조 변경이 좀 필요해서, 실제로 몇 분 이상 플레이해보고 계속 벌어지는 추세인지
확인한 뒤 착수할지 결정.
