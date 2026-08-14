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

---

# Part 6. 기능 구현 요약 (연료 · 대기권 · 행성간 이동 · 블랙홀 · HUD)

로드맵 "마무리" 단계에서 새로 구현한 것들. 이 아래(Part 7~)는 그 과정에서 만난 버그들.

## 연료
- `ASpaceship`에 `_fuel`/`_maxFuel`/`_fuelConsumeRate` 추가. 추력 중(`_isThrusting`)에만
  `deltaTime` 기준으로 소모, 0 이하로는 안 내려가게 클램프.
- `RefuelFull()` 추가 — HUD의 "Refuel" 버튼으로 즉시 완충.

## 대기권 연출 (`LaunchWorld`)
- 레일리 산란(β∝1/λ⁴) 상대 비율(R:G:B ≈ 0.27:0.61:1.0)에 대기 밀도(`expf(-altitude/scaleHeight)`)를
  곱해 하늘색을 물리 기반으로 유도 (`ComputeRayleighSkyColor`). 팔레트를 손으로 고르지 않고
  공식에서 뽑아냄.
- 별(`StarField`)이 고도에 따라 서서히 밝아짐(`1 - density`), 카메라 줌도 같은 `density` 값으로
  자동 축소 — "고도 하나가 하늘색·별·줌을 동시에 구동"하는 원래 로드맵 설계를 그대로 구현.
- `UPhysicsComponent::ComputeAcceleration()`에 대기 항력(`F ∝ v²`, 속도 반대 방향, 밀도에 비례) 추가.

## SOI(영향권) / 행성 간 이동
- 라플라스 SOI 공식 `r_SOI = a·(μ_planet/μ_parent)^0.4`로 각 행성의 영향권 반지름 계산
  (`APlanet::GetSOIRadius`).
- `ASpaceship::UpdateTargetPlanet()`이 매 프레임 현재 위치가 어느 행성의 SOI 안인지 판정해서
  `_targetPlanet`을 자동 전환. SOI 밖이면 `mu`가 제일 큰 천체(태양)로 귀속.
  경계에서 매 프레임 타겟이 바뀌는 걸 막기 위해 현재 타겟에만 5% 히스테리시스 부여.
- `OrbitalGameMode`에 `_home`(판정 기준 홈 행성)을 별도로 저장해서, 타겟이 홈 행성이 아닐 때는
  (=순항 중) 궤도 판정을 쉬게 함.

## 블랙홀
- `ABlackHole` 액터 신설(`EActorType::BlackHole`, `Planet`이 아님 — SOI의 "mu 제일 큰 천체 찾기"
  로직과 충돌 방지). 중력은 `UPhysicsComponent::ComputeAcceleration()`에서 SOI와 무관하게 항상 추가.
- `SpaceLightingSystem::ApplyBlackHoleLensing()` — 배경 별 왜곡(중력렌즈) + 강착원반 글로우 +
  이벤트 호라이즌(검은 코어)을 순서를 지켜 합성.
- 블랙홀 근처 별들(`_fallingStars`)이 블랙홀 주위를 공전하며 서서히 빨려들고(반경 감소 + 각도
  증가), 가까울수록 더 빠르게 도는 강착원반 흉내(`swirlSpeed ∝ 1/radius`). 이벤트 호라이즌에
  닿으면 바깥에서 재생성.
- 일반 별밭(`_starsWorld`)은 태양 중심으로 거리에 따라 감쇠하는 공전(가까울수록 빠르고, 천왕성
  거리 이후로는 거의 정지).
- 블랙홀의 화면 표시 크기(코어/글로우)에 최소·최대 클램프를 둬서, 줌아웃해도 아예 안 보이거나
  줌인했을 때 화면을 뒤덮지 않게 LOD 처리.

## HUD (`Widget_Main`)
- `MainWorld`에 새 위젯 신설 — 속도/근지점/원지점/상태/현재 중력원 텍스트, 연료 게이지
  (`UISlider` 재사용), Refuel 버튼, 우측 하단 방향 표시기(`UIHeadingIndicator`, 화살표 회전).
- 클릭으로 카메라 추적 타겟을 바꾸는 기능이 ImGui 패널이나 자체 UI 버튼 클릭에는 반응하지
  않도록 가드 추가.

---

# Part 7. 연료 시스템 버그

## 14. 연료 게이지를 켜자마자 우주선이 조작과 무관하게 계속 추력을 뿜음

**증상**
`W`를 안 눌러도 우주선이 계속 가속하고, 연료가 0이 돼도 추력이 안 꺼짐.

**원인**
```cpp
_isThrusting = _INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up);
...
_isThrusting = (_fuel >= 0.f);   // 바로 윗줄을 그냥 덮어씀 (대입이지 조건 결합이 아님)
```
키 입력 결과를 조건 결합(`and`)이 아니라 대입(`=`)으로 통째로 덮어써서, 이후로는 키 입력과
무관하게 `_fuel >= 0.f`만으로 추력 여부가 결정됨. 게다가 `_fuel`은 같은 프레임에
`clamp(_fuel, 0.f, _maxFuel)`로 하한이 0에 묶여있어서 **`_fuel >= 0.f`는 항상 참** — 연료가
정확히 0이어도 추력이 안 꺼짐.

**해결**
```cpp
_isThrusting = (_INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up))
               and (_fuel > 0.f);   // 대입이 아니라 결합, 등호 없이 엄격 부등호
```
> 📌 클램프로 하한이 묶인 변수의 "고갈됐다" 판정은 항상 그 경계와 **반대 방향의 엄격 부등호**여야
> 한다. `>=`를 쓰면 클램프가 이미 보장하는 걸 다시 체크하는 무의미한 조건이 된다.

---

## 15. 연료 소모를 넣을 자리를 잘못 고르면 소모 속도가 프레임 구조에 따라 널뛴다 (사전 방지)

물리 서브스텝(`UPhysicsComponent::PhysicsStep`)은 RK4일 때 한 스텝에 `ComputeAcceleration()`을
4번 부르고, 한 프레임에 최대 25스텝까지 돈다. 연료 소모를 그 함수 안에 넣으면 최악의 경우
프레임당 최대 100배 과소모될 수 있어서, 실시간 1프레임에 1번만 불리는 `ASpaceship::Update()`
(deltaTime 그대로) 쪽에 소모 로직을 뒀다. 실제 버그로 터지진 않았지만, "물리 가속도 계산"과
"게임 상태 변화"는 호출 빈도가 다른 자리에 있어야 한다는 걸 미리 점검한 사례.

---

# Part 8. 대기권 연출 버그

## 16. 하늘이 상승 시작하자마자 새까맣게 고정됨

**증상**
발사 직후부터 하늘이 서서히 어두워지는 게 아니라 처음부터 쭉 검음.

**원인**
```cpp
float normalizedAltitude = Normalize(_altitude, 0.f, 1.f);   // max 자리에 "목표 출력 범위"를 넣음
```
`Normalize(value, min, max)`는 `value`를 `[min,max]`에서 `[0,1]`로 매핑하는 함수인데, 여기
`max` 자리에 "결과가 0~1이어야 한다"는 출력 범위를 그대로 넣어버림. 실제 필요한 max는
"하늘이 완전히 우주가 되는 고도"였는데, 고도가 조금만 올라가도(`altitude > 1`) 내부
`clamp(normalized, 0, 1)`에 걸려 즉시 1.0으로 고정됨.

**해결**
```cpp
float normalizedAltitude = Normalize(_altitude, 0.f, kSpaceAltitude);   // 실제 핸드오프 고도 사용
```

---

## 17. 하늘색 기준 고도와 실제 우주 진입 고도가 서로 다른 상수를 참조

**증상**
16번을 고친 뒤에도 하늘이 너무 빨리 어두워짐.

**원인**
`LaunchWorld::Update()`의 핸드오프 판정(`if (_altitude > 10000.f)`)과 `Render()`의 하늘색
정규화 기준이 서로 다른 매직 넘버(10000 vs 1000)를 따로 들고 있었음 — 한쪽만 고치고 다른 쪽을
깜빡하기 쉬운 배치.

**해결**
같은 값을 참조하는 이름 있는 상수 하나로 통일(`kSpaceAltitude`). 이후 대기 항력 계산에 쓰는
"밀도가 1/e로 줄어드는 고도"도 `GAtmosphereScaleHeight`라는 전역 상수로 분리해서 `LaunchWorld`와
`UPhysicsComponent` 양쪽이 같은 값을 공유하게 함.

---

## 18. `LaunchWorld`에 별을 붙였는데 거의 안 보임

**증상**
`StarField::Init(count, range)`를 `MainWorld`에서 쓰던 값 그대로 복붙했더니 별이 거의 안 보임.

**원인**
`MainWorld`는 태양계 스케일(행성 간 거리 수만~수십만)이라 그 range가 맞았지만, `LaunchWorld`는
발사대 근처(고도 0~1만, 카메라 폭 수백~수천)라 스케일이 완전히 다름. 같은 개수의 별을 훨씬 넓은
범위에 흩뿌리니 밀도가 지나치게 낮아짐.

**해결**
`LaunchWorld` 스케일에 맞는 훨씬 작은 range로 재조정. (뒤에 블랙홀 별밭에서도 똑같은 종류의
실수 — 다른 스케일의 값을 그대로 복붙 — 가 반복됨, Part 11 참고.)

---

# Part 9. 행성 텍스처 태양광 방향 맞추기 — 성능 문제로 롤백

배경: 행성 이미지들이 전부 미리 3D로 라이팅되어 렌더링된 상태(고정된 하이라이트 방향)라, 행성이
공전해도 밝은 면이 항상 같은 화면 방향을 향해 보임 — 실제 태양 방향과 어긋남. `RenderRotated`로
태양 방향에 맞춰 스프라이트를 회전시키는 걸 시도했다가, 성능 문제로 최종적으로 롤백함.

## 19. 행성 회전을 넣자 fps가 10까지 떨어짐

**증상**
텍스처 회전을 추가하자마자 프레임이 10 근처로 폭락.

**원인**
`Texture::RenderRotated()`는(투명 배경 처리 시) 호출할 때마다 `CreateCompatibleDC` +
`CreateCompatibleBitmap`로 임시 버퍼를 새로 만들고 끝나면 지우는 구조. 우주선 하나만 쓸 때는
프레임당 1번이라 안 보였는데, 행성 9개가 전부 이 함수를 매 프레임 부르면서 GDI 객체를 초당
수백 개씩 생성/삭제.

**해결(1차)**
임시 DC/비트맵을 매번 새로 만들지 않고, 필요한 크기가 커질 때만 다시 만드는 캐시로 변경.
(`Texture`에 `_tempDC`/`_tempBitmap`/`_tempDim` 멤버 추가)

---

## 20. 캐시 도입 직후 화면이 계단식으로 깨짐

**증상**
행성들이 잘린 반원, 계단식 경계를 가진 이상한 모양으로 렌더링됨.

**원인**
```cpp
RECT r = { 0, 0, maxDim, maxDim };   // 이번 호출에 필요한 크기(작을 수 있음)
::FillRect(_tempDC, &r, bgBrush);
...
::TransparentBlt(hdc, drawX, drawY, _tempDim, _tempDim, ...);  // 캐시된 실제 버퍼 크기(더 클 수 있음)
```
버퍼를 지우고 중심을 잡는 데는 `maxDim`(이번 호출 값)을, 실제로 화면에 복사할 때는 `_tempDim`
(캐시된 버퍼의 실제 크기, 이전에 더 큰 행성 때문에 커져있을 수 있음)을 섞어 씀. 캐시가 재사용될
때(더 작은 행성을 그릴 때) 버퍼의 일부만 지워지고 중심도 잘못 잡힌 채로, 지워지지 않은 이전
내용까지 통째로 블릿돼서 잔상/계단 형태로 나타남.

**해결**
버퍼를 새로 만들지 판단하는 데만 `maxDim`을 쓰고, 그 이후(채우기·중심·블릿)는 전부 `_tempDim`
(실제 버퍼 크기) 기준으로 통일.

---

## 21. 결국 성능 문제로 행성 회전 자체를 롤백

**증상**
버퍼 재사용 최적화(19~20번)를 다 반영해도, 행성 9개가 매 프레임 `RenderRotated`를 부르는
비용 자체가 여전히 컸음(화면에 안 보이는 행성도 컬링 없이 전부 계산).

**해결**
`APlanet::Render()`를 회전 없는 `_texture->Render(...)`로 되돌림. 태양광 방향 맞추기는
보류하고, 대신 이미 있는 `ApplyPlanetShadow()`(실시간 그림자, 지금은 모든 행성에 적용 중)로
낮/밤 표현을 대신함.
> 📌 죽은 코드 정리: 이때 쓰던 `World::FindDominantPlanet()`(태양 찾기 헬퍼)은 더 이상 아무도
> 안 부르지만, 나중에 다른 조명 효과에 재사용할 수 있어서 남겨둠. 이름에 오타(`Palnet`)가
> 있었던 것도 발견 — 기능엔 지장 없지만 세 군데(선언·구현·호출) 다 같은 오타라 검색성이 나쁨.

---

# Part 10. 블랙홀 구현 — 가장 오래 걸린 디버깅 시리즈

## 22. 강착원반 글로우를 그리자마자 다음 줄이 다시 지워버림

**증상**
`ApplyBlackHoleLensing()`을 붙였는데 이벤트 호라이즌(검은 원)도, 강착원반 글로우도, 렌즈
왜곡도 전혀 안 보임. `AlphaBlend` 호출은 성공(`true`)을 반환하는데도 화면엔 아무것도 없음.

**원인**
```cpp
_accretionGlow->Render(_hDIBDC, bhPos, effectRadius * 0.8f, 220);   // ① 글로우를 그림

for (...) {
    if (dist < bhRadius) _pPixels[...] = 0;                         // ② 검게 덮어씀
    else if (dist < effectRadius) _pPixels[...] = _pBgPixels[...];  // ③ "그리기 전" 배경으로 덮어씀
}
```
②③ 루프가 훑는 범위(`0~effectRadius`)가 ①에서 글로우를 그린 범위(`0~effectRadius*0.8`)를
완전히 포함해서, 글로우를 그리자마자 바로 다음 코드가 그 자리를 검정이나 "글로우 그리기 전"
배경으로 덮어써버림. 좌표 계산은 전부 맞았는데, **그리는 순서**가 문제였음.
> 📌 이걸 확인하려고 좌표 검증용 마젠타 원(순수 GDI, DIB 파이프라인 완전 우회)을 임시로
> 그려봐서 "좌표는 맞다"를 먼저 증명한 뒤에 순서 문제라는 걸 좁혀나감.

**해결(1차)**
글로우를 왜곡 루프 **뒤**로 옮김. → 이번엔 23번 문제가 생김.

---

## 23. 글로우를 맨 뒤로 옮기니 이번엔 이벤트 호라이즌(검은 코어)이 안 보임

**증상**
글로우는 보이는데, 화면 전체가 빛나는 별처럼 보이고 한가운데 있어야 할 검은 원이 없음.

**원인**
글로우를 마지막에 그리면서, 글로우의 밝은 중심(반경 `effectRadius*0.8`)이 검은 코어가 있어야
할 자리(반경 `bhRadius`)까지 덮어버림 — 그리는 순서를 통째로 뒤집으면서 다른 층을 밟은 것.

**해결**
3단계로 순서를 명확히 함: **① 배경 왜곡 → ② 강착원반 글로우 → ③ 검은 코어를 맨 마지막에
다시 뚫기.** 이렇게 하면 각 층이 서로 안 지운다.

---

## 24. 렌즈 왜곡이 있는지 없는지 티가 안 남

**증상**
효과가 적용된 건 맞는데 눈에 거의 안 띔.

**원인**
`distortionStr` 기본값이 15인데, 호출부에서 인자 2개만 넘겨서 그 기본값을 그대로 씀. 별
사이 간격에 비하면 15px 정도의 왜곡은 거의 안 느껴짐.

**해결**
`distortionStr`을 100 안팎으로 직접 넘기도록 호출부 수정. 추가로 `effectRadius` 경계에서
왜곡 강도가 15%쯤 남은 채로 뚝 끊겨서 생기는 경계선은, 바깥쪽 30% 구간에서 부드럽게 0으로
수렴하는 페이드를 곱해서 제거.

---

## 25. `AddBlackHoleFallingStars()`의 별 반경 생성식 오타 (`*` ↔ `+`)

**증상**
블랙홀 근처 별 무리(2만 개)가 거의 다 화면에 안 보이거나 멈춰있는 것처럼 보임.

**원인**
```cpp
s.radius = eventRadius * ((float)rand() / RAND_MAX) * (maxRadius - eventRadius);
```
더하기(`+`)여야 할 자리가 곱하기(`*`)로 되어있어서, `eventRadius=5000 maxRadius=15000` 기준
결과가 **0~5천만** 사이로 나옴(의도한 5000~15000이 아니라). 낙하 속도(800/s) 대비 반경이
너무 커서 화면 안으로 들어오는 데 17시간 넘게 걸림 — 사실상 멈춰있는 것처럼 보임.

**해결**
```cpp
s.radius = eventRadius + ((float)rand() / RAND_MAX) * (maxRadius - eventRadius);
```

---

## 26. `maxRadius`를 `eventRadius`보다 작게 줄여서 별들이 코어 안쪽에 갇힘

**증상**
띠를 얇게 만들려고 반경을 확 줄였더니, 줌을 아무리 바꿔도 별이 아예 안 보임.

**원인**
`maxRadius(1000) < eventRadius(5000)`으로 역전됨. 이러면 별 반경 생성식 결과가 1000~5000
사이로 나오는데, 재생성 조건(`fs.radius < eventRadius`)을 거의 항상 만족해서 **매 프레임 즉시
`radius = maxRadius(=1000)`로 리셋**됨 — 별들이 전부 이벤트 호라이즌(반경 5000)보다 훨씬 안쪽인
반경 1000에 갇혀서, 나중에 그려지는 검은 코어에 항상 덮여 안 보임.

**해결**
`maxRadius`는 항상 `eventRadius`보다 커야 함(띠를 얇게 하려면 `eventRadius`에 더 가깝게 줄이되,
역전은 안 됨).

---

## 27. 별 띠가 다시 왜곡 루프에 지워짐 (22번과 같은 종류, 다른 대상)

**증상**
26번을 고쳐도 여전히 별 띠가 안 보임.

**원인**
별 띠를 `BeginRender()`(왜곡 루프보다 먼저 실행)에서 그렸는데, 띠가 너무 좁게(반경
`eventRadius`~`1.4×eventRadius` 정도) 몰려있다 보니 `ApplyBlackHoleLensing()`의 왜곡 루프가
그 좁은 범위를 지나쳐서 더 바깥의 빈 공간을 샘플링해오는 경우가 대부분 → 22번과 똑같은
"먼저 그린 걸 나중 단계가 덮어씀" 패턴이 별 띠에도 반복됨.

**해결**
별 띠 그리는 코드를 `BeginRender()`에서 빼서 `ApplyBlackHoleLensing()` 안, **왜곡 루프 뒤 ·
검은 코어 뚫기 전**으로 옮김(22~23번에서 정리한 3단계 순서에 맞춤).

---

## 28. 블랙홀 코어는 화면 크기가 고정(클램프)되는데 별 띠는 카메라 줌을 그대로 따라감

**증상**
줌인할수록 코어 크기는 그대로인데 주변 별 고리만 점점 커져서 따로 노는 것처럼 보임.

**원인**
코어/글로우 크기는 "너무 작아지거나(줌아웃) 너무 커지는(줌인) 것"을 막으려고 화면 픽셀
기준 최소/최대로 클램프해뒀는데, 별 띠는 여전히 카메라의 실제 줌(`cam.WorldToScreen`)으로
위치를 계산해서 클램프의 영향을 안 받음 — 서로 다른 스케일 기준을 쓰고 있었음.

**해결**
별의 화면 위치도 카메라의 실제 줌이 아니라, 코어에 적용한 것과 **같은 보정 배율**
(`bhScale = 클램프된 화면 반경 / eventRadius`)로 계산하도록 통일.

---

## 29. 줌인하면 글로우가 화면 전체를 뒤덮어 뭉개진 흰 안개처럼 보임

**증상**
어느 정도 줌인하니 주황빛도 별 줄무늬도 다 사라지고 그냥 부옇게 뭉개진 흰색만 남음.

**원인**
`effectRadius = bhRadius * 3.5`가 화면 픽셀 기준으로 계산되는데, 코어 크기에 최소 클램프만
있고 **최대 클램프가 없어서** 줌인할수록 끝없이 커짐 — 확인해보니 `effectRadius`가 800px
안팎까지 커져서 600×800 화면 전체를 덮어버림. 글로우 텍스처를 그 정도로 초확대 스트레치하면
바깥쪽(거의 투명한) 부분만 화면에 보여서 뭉개진 흰색으로 보임.

**해결**
코어 화면 반경에 최소뿐 아니라 **최대**도 클램프(`clamp(r, 15.f, 80.f)`).

---

# Part 11. GDI 리소스 관리

## 30. 임시 버퍼를 키우는 코드에서 선택된 비트맵을 먼저 삭제 (정의되지 않은 동작)

**증상**
캐시 버퍼가 커져야 하는 상황(더 큰 행성을 그릴 때)에서 화면이 이상하게 깨짐.

**원인**
```cpp
if (_tempBitmap) { ::DeleteObject(_tempBitmap); _tempBitmap = nullptr; }   // 아직 _tempDC에 선택된 채로 삭제
if (_tempDC)     { ::DeleteDC(_tempDC); _tempDC = nullptr; }
```
`_tempBitmap`이 `_tempDC`에 **아직 선택된 상태에서** 먼저 삭제됨 — GDI에서 선택된 비트맵을
바로 지우는 건 정의되지 않은 동작. 원래 있던 `Texture` 소멸자에는 이미 정답
(`// DC를 먼저 삭제하면 선택되어 있던 비트맵이 자동으로 해제된다`)이 주석으로 적혀 있었는데
그 순서를 안 따름.

**해결**
```cpp
if (_tempDC)     { ::DeleteDC(_tempDC); _tempDC = nullptr; }        // DC를 먼저 지워서 자동 deselect
if (_tempBitmap) { ::DeleteObject(_tempBitmap); _tempBitmap = nullptr; }
```

---

# Part 12. UI/HUD 버그

## 31. 버튼에 호버하면 텍스트만 커지고 위치는 안 따라가서 이미지와 어긋남

**증상**
버튼(`UIButton`) 위에 마우스를 올리면 글자 크기는 커지는데 위치가 버튼 이미지와 안 맞음.

**원인**
`UIButton::SetText()`가 텍스트의 부모 기준 위치(`SetParentAnchor`)를 **한 번만** 계산해두는데,
호버로 버튼이 커지면(`SetSize`) 버튼의 실제 크기/위치는 바뀌어도 텍스트는 처음 계산해둔 위치에
그대로 남음.

**해결**
`SetText()` 호출 시 인자(anchor/pivot/pos/size)를 멤버로 저장해두고, `Update()`의 호버/언호버
분기에서 버튼 `SetSize()` 직후 텍스트도 같은 인자로 `InitButton()`을 다시 호출해서 위치를
재계산.

---

## 32. `SetText()`에 커스텀 폰트 크기를 넘겨도 호버하면 원래 크기로 돌아옴

**증상**
버튼 글씨를 작게 만들었는데, 마우스가 한 번 올라갔다 내려오면 다시 커짐.

**원인**
```cpp
// UIButton::Init()
_originFontSize = 40.f;   // 무조건 40 고정

// UIButton::SetText()
if (fontSize <= 0.01) _text->SetFontSize(_originFontSize);
else _text->SetFontSize(fontSize);   // 처음엔 커스텀 크기로 그리지만
```
`Update()`의 호버 로직은 `_originFontSize`(여전히 40) 기준으로 계속 리셋하기 때문에, 커스텀
크기는 처음 한 프레임만 반영되고 호버 즉시 원래 40으로 돌아감.

**해결**
```cpp
if (fontSize > 0.01f) _originFontSize = fontSize;   // 커스텀 크기를 기준값 자체에도 반영
```

---

## 33. 카메라 추적 타겟이 ImGui/자체 UI 클릭에도 풀림

**증상**
ImGui 디버그 패널이나 자체 HUD 버튼(Refuel 등)을 클릭해도, 게임 월드의 빈 우주를 클릭한 것과
똑같이 카메라 추적 타겟이 풀림.

**원인 (1) ImGui 미체크**
```cpp
if (_INPUT.GetButtonDown(KeyType::LeftMouse))
{
    Vector2 worldPos = _camera.WorldToMousePos(_INPUT.GetMousePos());
    _selected = PickActor(worldPos);
    _camera.SetFollowTarget(_selected);   // PickActor가 nullptr이면 그대로 타겟 해제
}
```
마우스가 ImGui 패널 위에 있는지, 게임 뷰포트 안인지 체크가 아예 없어서 어디를 클릭해도
`PickActor`(그 자리에 액터가 없으면 `nullptr`)를 그대로 `SetFollowTarget`에 넘김.

**원인 (2) 자체 UI 판정 실수**
```cpp
bool overOwnUI = _widget and _widget->IsHoverInUI(mousePos);   // Widget "자기 자신"의 크기 체크
```
`Widget_Main`은 자기 자신은 크기를 설정한 적이 없어서(자식들만 배치) 사실상 크기 0인 상자를
체크 — 항상 `false`가 나와서 가드가 한 번도 작동하지 않음.

**원인 (3) 좌표계 불일치 (일부)**
`_INPUT.GetMousePos()`는 실제 윈도우 픽셀 좌표인데, UI 요소들은 논리 해상도(`GWinSizeX`×
`GWinSizeY` = 600×800) 기준으로 배치됨 — 창 크기가 정확히 600×800이 아니면 어긋남.

**해결**
```cpp
// Widget에 자식들을 순회하는 함수 추가
bool Widget::IsMouseOverUI(Vector2 mousePos) const
{
    for (UIBase* child : _children)
        if (child->IsHoverInUI(mousePos)) return true;
    return false;
}

// UIBase::IsHoverInUI에 좌표 보정 추가
Vector2 scaledMouse = mousePos / GAME.GetRectRatio();

// MainWorld::Update()
if (_INPUT.GetButtonDown(KeyType::LeftMouse))
{
    ImGuiIO& io = ImGui::GetIO();
    Vector2 mousePos = _INPUT.GetMousePos();
    bool overOwnUI = _widget and _widget->IsMouseOverUI(mousePos);

    if (_INPUT.IsMouseInsideWindow(GAME.GetGameViewportRect()) and not io.WantCaptureMouse and not overOwnUI)
    {
        Vector2 worldPos = _camera.WorldToMousePos(mousePos);
        _selected = PickActor(worldPos);
        _camera.SetFollowTarget(_selected);
    }
}
```
> 📌 "빈 우주를 클릭하면 추적이 풀린다"는 의도된 기능이라(자유 카메라로 전환하는 용도),
> "액터를 클릭했을 때만 타겟을 바꾼다"로 조건을 바꾸는 대신, "ImGui/자체 UI를 클릭한 경우만
> 예외 처리"하는 방향으로 감. 기능은 유지하면서 오작동만 막음.

---

# Part 13. 기능 구현 요약 (행성간 자동 전이)

로드맵 "마무리" 단계의 마지막 큰 기능. "우주선을 피킹 → 행성 버튼 생성 → 클릭하면 궤도 계산해서
자동으로 날아감"을 목표로, 필요한 물리(케플러 궤도, 람베르트 문제)부터 공부하면서 구현함.
이 아래(Part 14~)는 그 과정에서 만난 버그들.

## 케플러 타원 궤도 (`APlanet`)
- 기존 원궤도(`_orbitRadius`, 각도만 증가)를 실제 이심률이 있는 타원 궤도로 교체.
  `_meanAnomaly`(M, 시간에 따라 균일 증가) → `SolveKeplerEquation`(뉴턴법 6회 반복, `E = M + e·sinE`가
  초월방정식이라 대수적으로 못 풀어서)로 이심 근점이각(E) → 진근점이각(ν) → 위치·속도 순으로 변환.
- `GetOrbitShape(steps)`는 시간과 무관하게 E를 0~2π로 직접 훑어서 궤도 "모양"만 그림 (케플러 방정식을
  풀 필요가 없음 — 모양은 시간에 안 달림).

## 람베르트 문제 (`Util::SolveLambert` → 이후 `Lambert.h/.cpp`로 분리)
- 케플러 방정식의 역문제: 두 위치(r1, r2)와 이동 시간(TOF)을 주면 그 둘을 잇는 데 필요한 속도(v1, v2)를
  구함. Universal Variable 방법(Stumpff 함수 C(z)/S(z), z에 대한 이분법)으로 구현.
- 이분법을 뉴턴법 대신 고른 이유: 버튼 한 번 누를 때 한 번만 도는 계산이라 수렴 속도보다 안정성이
  더 중요했음.

## TOF 스캔 (`InterplanetaryTransfer::FindBestTransfer`)
- 호만 전이 시간을 대략적인 기준으로 잡고, 그 0.3~2.5배 범위에서 TOF 후보들을 훑어 람베르트를 반복
  실행 → 총 Δv(출발+도착)가 최소인 조합을 선택. coarse(20개)로 넓게 훑고 그 근처를 fine(80개)으로
  촘촘하게 다시 훑는 2단계 탐색으로, 같은 샘플 예산으로 정밀도를 끌어올림.

## 우주선 피킹 → 자동 실행 → 시각화
- 우주선을 클릭(또는 ImGui 트리에서 더블클릭)하면 `Widget_Main`에 행성 버튼들이 뜨고, 누르면
  `FindBestTransfer`로 계산한 속도(`plan.v1`)를 즉시 우주선에 적용.
- 계산된 궤적을 `PredictTransferPath`(태양 중심 2체 문제만 가정한 별도 RK4 전파 함수)로 미리 그려서
  화면에 점선으로 표시.

---

# Part 14. 케플러 타원 궤도 버그

## 34. 행성들이 전부 겹쳐 보임 (위치 계산에서 반지름 곱셈 누락)

**증상**
타원 궤도로 바꾼 직후, 모든 행성이 태양 바로 옆 한 점에 겹쳐서 보임.

**원인**
```cpp
Vector2 APlanet::ComputePositionAtEccentricAnomaly(float E) const
{
    float nu = ...;
    float r = _semiMajorAxis * (1.f - _eccentricity * cosf(E));
    return _orbitCenter + Vector2(cosf(nu + _argPeriapsis), sinf(nu + _argPeriapsis));   // ← r 안 곱함
}
```
방향(단위벡터)만 계산하고 실제 거리(`r`)를 곱하는 걸 빠뜨려서, 모든 행성이 태양에서 반지름 1
거리에 위치하게 됨.

**해결**
```cpp
return _orbitCenter + Vector2(cosf(nu + _argPeriapsis), sinf(nu + _argPeriapsis)) * r;
```

---

## 35. `Setup()`에서 이심률/근일점 인자를 멤버에 대입 안 함

**증상**
34번을 고쳐도 궤도가 여전히 완벽한 원 — 이심률을 줘도 반영이 안 됨.

**원인**
`Setup()`이 `eccentricity`/`argPeriapsis`를 매개변수로는 받으면서, 정작 `_eccentricity`/`_argPeriapsis`
멤버 변수에는 대입하는 줄이 빠져있었음(둘 다 기본값 0으로 남음 → 원운동 공식과 동일해짐).

**해결**
```cpp
_eccentricity = eccentricity;
_argPeriapsis = argPeriapsis;
```
> 📌 검증은 ImGui로 실시간 거리 숫자를 찍어서 확인(수성 기준 근일점~원일점 15880~24120 사이를
> 오가는 것 확인) → 이후 눈으로도 "태양이 궤도 링 안에서 한쪽으로 치우쳐 보이는지"로 재확인.
> 이심률이 작을 때(수성 e=0.206)는 궤도 "모양"(장단축비 `b/a=√(1-e²)`≈97.9%)은 원이랑 거의
> 구분 안 되지만, 초점 오프셋(`c=a·e`≈20.6%)은 뚜렷이 보인다는 걸 직접 확인하며 배움.

---

# Part 15. 호만 전이 시간 공식 오타 — 이번 시리즈에서 가장 파급력 컸던 버그

## 36. `ComputeHohmannTOF`의 장반경 공식이 덧셈 대신 곱셈 (스캔 범위가 수백만 배 벌어짐)

**증상**
행성 버튼을 눌러도 `FindBestTransfer`가 계속 `valid=false`를 반환 — 전이 궤적이 아예 안 그려짐.
(처음엔 UI 연동 버그로 의심하고 그쪽을 먼저 다 고쳤는데도 재현됨.)

**원인**
```cpp
static float ComputeHohmannTOF(float r1, float r2, float mu)
{
    float a_t = (r1 * r2) * 0.5f;   // ← + 여야 할 자리에 *
    return 3.14159265f * sqrtf(a_t * a_t * a_t / mu);
}
```
호만 전이 궤도의 장반경은 두 반지름의 **평균**(`(r1+r2)/2`)인데 곱(`r1*r2`)으로 되어있었음.
지구(반지름 60000)→금성(반지름 40000) 기준으로 계산해보면:
- 맞는 값: `a_t=(60000+40000)/2=50000` → `TOF≈654.5`
- 버그 값: `a_t=(60000*40000)/2=12억` → `TOF≈24억` (약 370만 배)

`FindBestTransfer`가 스캔하는 TOF 범위(`hohmannTOF`의 0.3~2.5배)가 통째로 실제 스케일보다
수백만 배 큰 값으로 잡혀서, `SolveLambert`의 이분법이 그 범위 안에서 유효한 해를 못 찾고 계속
`invalid`를 반환함.

**해결**
```cpp
float a_t = (r1 + r2) * 0.5f;
```
> 📌 이 파일(`InterplanetaryTransfer.cpp`)의 초기 뼈대를 만들 때부터 있던 버그로, 케플러 궤도 검증
> 때처럼 눈에 보이는 이상(모양이 이상함)이 아니라 "그냥 아무 결과도 안 나옴"이라 원인 후보를
> 좁히는 데 오래 걸림 — 결국 관련 함수를 하나씩 다시 읽으면서 발견.

---

# Part 16. 행성 버튼 UI 연동(Step5) 리뷰 중 발견된 버그들

## 37. `ShowPlanetButtions`가 버튼 배열 크기만큼 도는데 그 안에서 `planets[i]`를 읽음

**증상 (코드 리뷰 중 발견)**
```cpp
for (int i = 0; i < kMaxPlanetButtons; ++i)   // 버튼 배열 크기(9) 기준
{
    wstring text = StringToWString(planets[i]->GetName());   // planets는 호출자가 넘긴 벡터
    ...
}
```
버튼 배열 크기(`kMaxPlanetButtons`)와 실제로 넘어온 목적지 목록(`planets`)의 크기가 같다는 보장이
없음 — 지금은 우연히 행성 개수와 버튼 개수가 같아서 안 터졌지만, 목적지 필터링(현재 위치한 행성
제외 등)을 넣는 순간 범위를 벗어난 메모리를 읽게 됨.

**해결**
```cpp
int count = min((int)planets.size(), kMaxPlanetButtons);
for (int i = 0; i < count; ++i) { ... }
for (int i = count; i < kMaxPlanetButtons; ++i) { /* 남는 버튼은 꺼두기 */ }
```

---

## 38. 매핑·클릭 콜백 자체가 통째로 빠짐

**증상**
버튼에 행성 이름은 뜨는데, 눌러도 아무 반응이 없음.

**원인**
TODO로 남겨뒀던 "버튼 인덱스 → 행성 매핑 저장"과 "`SetOnClick`에서 콜백 호출" 두 줄이 처음
구현에서 빠짐 — 텍스트만 채우고 정작 클릭 시 할 일을 안 넣음.

**해결**
```cpp
_planetButtonTargets[i] = planets[i];
_planetButtons[i]->SetOnClick([this, i]()
    {
        if (_onPlanetSelected) _onPlanetSelected(_planetButtonTargets[i]);
    });
```
> 📌 람다에서 `i`를 값으로 캡처(`[this, i]`)하는 게 중요 — 참조로 캡처(`[this, &i]`)하면 반복문이
> 끝난 뒤의 `i` 마지막 값 하나를 모든 버튼이 공유하게 됨(자바스크립트 `var` 클로저 문제와 동일한
> 종류의 실수).

---

## 39. `Widget::IsMouseOverUI`가 비활성(`SetActive(false)`) 버튼도 그대로 판정 — 숨긴 버튼 자리가 클릭 사각지대가 됨

**증상**
평소(버튼이 안 보이는 상태)에도 화면 우측 상단(행성 버튼들이 배치된 자리) 근처를 클릭하면
행성/블랙홀 피킹이 안 됨.

**원인**
```cpp
bool Widget::IsMouseOverUI(Vector2 mousePos) const
{
    for (UIBase* child : _children)
        if (child->IsHoverInUI(mousePos)) return true;   // _isActive 체크 없음
    return false;
}
```
33번에서 고친 `IsMouseOverUI`가 활성 상태는 확인하지 않고 자식들의 화면 좌표 범위만 검사함.
행성 버튼 9개는 안 보이는 동안에도(`SetActive(false)`) `_children`에 계속 남아있어서, 그 자리를
클릭하면 "내 UI를 클릭했다"고 오판해 월드 피킹으로 안 넘어감.

**해결**
```cpp
for (UIBase* child : _children)
{
    if (!child->IsActive()) continue;   // 추가
    if (child->IsHoverInUI(mousePos)) return true;
}
```
> 📌 33번과 같은 함수의 같은 종류의 문제가 새 사용처(토글되는 버튼)에서 다시 드러난 사례 — 처음
> 고칠 때는 "항상 켜져 있는 UI"만 대상이라 안 드러났던 전제(활성 상태 무시)가, 나중에 "껐다 켜는
> UI"가 추가되면서 진짜 버그로 발현됨.

---

# Part 17. 전이 궤적 시각화(Step6) 버그

## 40. RK4 전파 함수에 `UPhysicsComponent` 전용 코드가 그대로 딸려와 컴파일 에러

**증상**
`PredictTransferPath` 작성 후 빌드 실패.

**원인**
```cpp
pos = newPos;
vel = newVel;

GetOwner()->SetCenterPos(newPos);   // UPhysicsComponent::PhysicsStep에서 그대로 복사해온 줄
_velocity = newVel;                  // 이 함수(자유 함수)엔 GetOwner()도 _velocity도 없음
```
`UPhysicsComponent::PhysicsStep`의 RK4 계산을 참고해서 옮기는 과정에서, 그 함수가 컴포넌트
멤버라서만 유효했던 두 줄(`GetOwner()`, `_velocity`)까지 그대로 딸려옴.

**해결**
두 줄 삭제 — 바로 위 `pos = newPos; vel = newVel;`로 이미 로컬 상태 갱신이 끝나있었음.

---

## 41. 함수 이름이 선언부와 정의부에서 미묘하게 달라 컴파일 에러

**증상**
`vPredictTransferPath`로 정의해뒀는데 호출부에서는 `PredictTransferPath`로 불러서 "정의 안 된
식별자" 에러.

**원인**
`vector<Vector2>` 타이핑 도중 실수로 함수 이름 맨 앞에 `v`가 붙은 채로 굳어짐 — 처음엔 "기능엔
지장 없다"고 넘어갔는데, 이후 호출부를 작성할 때 원래 의도했던 이름(`PredictTransferPath`)으로
불러서 불일치가 발생.

**해결**
선언·정의·호출부 세 곳 모두 `PredictTransferPath`로 통일.

---

## 42. 궤적 시작 위치를 우주선 현재 위치가 아니라 "도착 지점"으로 넣음

**증상**
전이 버튼을 눌러도 궤적 점선이 우주선이 아니라 전혀 다른 곳에서 시작해서 엉뚱한 방향으로 그려짐.

**원인**
```cpp
_transferPath = PredictTransferPath(plan.arrivalPos, plan.v1, _sun->GetCenterPos(), _sun->GetMu(), plan.tof, 200);
//                                   ^^^^^^^^^^^^^^^ 도착 지점을 출발 위치 자리에 넣음
```
속도(`plan.v1`)는 맞게 넣었는데 첫 인자(출발 위치)에 우주선의 현재 위치 대신 `plan.arrivalPos`
(목적지가 도착 시점에 있을 자리)를 넣어서, "목적지에서 v1 속도로 출발하면 어디로 가는지"라는
전혀 다른 궤적을 그리고 있었음.

**해결**
```cpp
_transferPath = PredictTransferPath(_ship->GetCenterPos(), plan.v1, _sun->GetCenterPos(), _sun->GetMu(), plan.tof, 200);
```

---

# Part 18. 람베르트의 물리 가정과 실제 SOI 물리의 불일치

## 43. 계산된 궤적대로 안 가고 출발 행성 중력에 휘어짐 ("지 멋대로 가는" 것처럼 보임)

**증상**
전이를 실행하면 우주선이 계산된 궤적(흰 점선)을 안 따라가고, 출발 지점 근처에서 갈고리처럼
휘어진 뒤 엉뚱하게 날아감. (36번 호만 공식을 고친 뒤에도 재현.)

**원인**
`SolveLambert`가 계산한 `plan.v1`은 "출발부터 도착까지 태양 중력만 작용한다"는 순수 2체 문제를
가정한 값인데, 실제로 그 속도를 받는 `UPhysicsComponent::ComputeAcceleration()`은 그렇게 동작하지
않음 — `ship->GetTargetPlanet()`(SOI 판정으로 매 프레임 갱신되는, "지금 가장 영향력 큰 천체")의
중력만 적용함. 버튼을 눌러 속도만 바뀌는 순간 우주선은 여전히 출발 행성의 SOI 안(위치는 안
바뀌므로)에 있어서, 물리 엔진은 태양이 아니라 출발 행성 중력으로 가속시킴 — 람베르트의 가정과
실제 물리가 어긋남.

**해결**
`ASpaceship`에 "강제 헬리오센트릭" 모드를 추가 — 전이 실행 시점에 `SetTargetPlanet(_sun)`으로
강제 고정하고, `plan.tof` 동안은 `UpdateTargetPlanet()`의 자동 SOI 판정을 건너뛰게 타이머로 막음.
```cpp
// ASpaceship::UpdateTargetPlanet()
if (_forceHeliocentric) return;   // 강제 모드 중엔 자동 판정 스킵
```
타이머가 끝나면 자동 판정이 다시 켜지고, 그 시점엔 실제 물리(RK4)가 태양 중력만 받으며 진행돼
있어서 `PredictTransferPath`(똑같은 RK4·똑같은 태양 중력으로 미리 계산한 경로)와 실제 궤적이
거의 일치하게 됨.
> 📌 애초에 "SOI 밖(태양이 타겟일 때)에만 전이 버튼 활성화"로 막는 방안도 고려했지만, "지구를
> 돌고 있어도 버튼을 누르면 그 즉시 목적지로 날아가야 한다"는 요구사항이라 게이트 방식 대신
> 이 강제 전환 방식을 선택함.

---

## 44. 목적지에 도착해도 궤도에 안착하지 않고 스쳐 지나감

**증상**
43번을 고쳐서 계산된 경로대로 날아가는 것까지는 되는데, 목적지 행성 근처에서 궤도에 안착하지
않고 옆을 스쳐 지나감(플라이바이).

**원인**
`FindBestTransfer`/`ScanTOFRange`가 총 Δv(`dv1+dv2`)를 최소화하는 과정에서 "도착 시 필요한
감속량"(`dv2`)까지는 계산하지만, `TransferPlan`엔 출발 속도(`v1`)만 저장하고 도착 속도는 아예
저장하지 않음 — 그래서 실제로 적용되는 건 출발 버튼(`plan.v1`)뿐이고, 도착 시 목적지 행성 속도에
맞춰주는 두 번째 버튼이 없음. 게다가 람베르트가 겨냥하는 지점은 목적지 행성의 **중심**이라, 설령
속도까지 정확히 맞춘다 해도 "궤도 진입"이 아니라 "충돌/겹침"이 되어 애초에 궤도 매칭 방식과는
맞지 않음.

**해결**
정직한 속도 매칭 대신, **목적지 SOI 진입을 감지해서 그 자리에서 원궤도 속도로 스냅**시키는 방식을
채택(기존 `KEY_0` 디버그 코드의 "원궤도 속도 = √(μ/r), 반지름에 수직 방향" 패턴 재사용).
```cpp
// MainWorld::Update()
if (_transferTarget and not _transferCaptured and _ship->GetTargetPlanet() == _transferTarget)
{
    Vector2 toShip = _ship->GetCenterPos() - _transferTarget->GetCenterPos();
    float r = max(toShip.Length(), 1.f);
    Vector2 dir = toShip / r;
    Vector2 perp(-dir.y, dir.x);
    float speed = sqrtf(_transferTarget->GetMu() / r);

    physics->SetVelocity(perp * speed + _transferTarget->GetVelocity());
    _transferCaptured = true;
}
```
`_forceHeliocentric` 타이머가 끝나 자동 SOI 판정이 재개되면서 `GetTargetPlanet()`이 목적지
행성으로 바뀌는 순간을 감지해, 그 즉시(어느 거리에서 잡히든 그 거리 기준으로) 원궤도 속도로
스냅시킴. 순간적인 속도 변경이라 완전히 "사실적"이진 않지만, 이 프로젝트의 다른 모든 속도 변경
(추력, 디버그 텔레포트, 출발 버튼 자체)도 전부 같은 임펄스 방식이라 일관성은 있음.
> 📌 도착 이후(궤도에 스냅된 뒤)는 다시 플레이어의 수동 조작(추력)으로 넘어감 — 자동화는
> "행성간 전이"라는 계산이 어려운 구간만 담당하고, 착륙/정밀 궤도 조정처럼 손맛이 필요한 부분은
> 그대로 플레이어 몫으로 남겨두는 설계.
