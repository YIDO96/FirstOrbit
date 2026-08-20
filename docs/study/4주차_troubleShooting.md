# 4주차 트러블슈팅 — 은하 공전 · 시간 배속 · 위성

> 3주차(궤도 게임화·마무리) 이후, 로드맵에 "확장 아이디어로 기록만" 해두었던 항목들
> (태양계 자체의 움직임, 위성)을 실제로 건드리면서 터진 버그들을 증상 → 원인 → 해결
> 순으로 남긴다. 시간 배속(타임워프) 기능은 이번에 처음 추가됐고, 그 과정에서
> "배속이 커질수록만 드러나는" 종류의 버그가 여러 개 연쇄로 나왔다는 게 이번 주의 특징이다.

---

# Part 1. 태양을 은하 중심 궤도로 전환

배경: 그동안 태양은 `(0,0)`에 완전히 고정돼 있었고, "태양계가 은하를 이동한다"는 연출은
카메라 전용의 가짜 `_galacticZ` 축(사이드뷰 토글용, 실제 월드 좌표엔 영향 없음)으로만
흉내내고 있었다. 이번에 태양도 다른 행성과 똑같은 `APlanet::Setup()`을 재사용해서, 아주
멀리 떨어진 가상의 은하 중심(`galacticCenter`, 반지름 3,000,000, 각속도 0.00003rad/s)을
실제로 도는 궤도로 바꿨다.

## 1. 우주선 스폰 직후 지구를 전혀 못 돎 (Enter()가 Update()보다 먼저 실행되는 순서 문제)

**증상**
LaunchWorld에서 MainWorld로 넘어오면 우주선이 지구를 공전하지 않고 그대로 직진/이탈해버림.

**원인**
행성이 "궤도 중심 몸체 자체가 움직이면 그 가속도/속도까지 이어받는" 체인
(`APlanet::_orbitCenterBody`, `GetAcceleration()`/`GetVelocity()`가 재귀적으로 합산)을
새로 만들었는데, 이 체인을 매 프레임 연결해주는 코드가 `MainWorld::Update()` 안에만 있었다.
문제는 `MainWorld::Enter()`가 우주선 초기속도를
`handoff.velocity + _homePlanet->GetVelocity()`로 계산하는 시점은 **`Update()`가 한 번도
실행되기 전**이라, 그 순간 지구의 `_orbitCenterBody`는 아직 `nullptr`다. 그래서
`GetVelocity()`가 태양 자신의 은하 공전 속도(약 90 units/s)를 빼먹은 채로 반환된다.

숫자로 보면 왜 치명적인지 드러난다: 태양의 은하 공전 속도(~90)를 저궤도(반지름 1300)
기준 필요 속도(`√(μ/r) ≈ 124`)와 비교하면 **72%에 달하는 오차**다. "고작 90"이 아니라
국소 궤도속도의 대부분을 차지하는 크기라, 궤도가 찌그러지는 정도가 아니라 통째로 깨져서
그대로 이탈한다.

**해결**
`InitPlanet()`에서 행성들을 다 만든 직후, 블랙홀을 스폰하기 전에 한 번 미리 체인을
연결해준다.
```cpp
// MainWorld::InitPlanet(), 태양계 블록이 끝난 직후
for (auto planet : _planets)
{
    if (planet == _sun) continue;
    planet->SetOrbitCenter(_sun->GetCenterPos(), _sun);
}
```
`Update()` 쪽의 매 프레임 갱신 루프는 그대로 두되(태양 위치 자체는 계속 갱신해야 하므로),
이 한 번의 사전 연결 덕분에 `Enter()`가 실행되는 시점엔 이미 체인이 살아있다.

> 📌 이 체인(`_orbitCenterBody`)은 나중에 Part 4의 위성 기능을 만들 때 그대로 재사용됐다 —
> "달이 지구를 돈다"는 "지구가 태양을 돈다"와 구조적으로 완전히 같은 문제라, 새 클래스나
> 새 물리 없이 같은 메커니즘을 한 번 더 얹기만 하면 됐다.

---

# Part 2. 시간 배속(타임워프)

배경: `x1 / x10 / x20 / x100` 버튼을 추가해서, `WORLD.Update(deltaTime * _timeScale)`로
게임 시간 자체를 빨리 돌리는 기능을 넣었다. 배속을 걸자마자 연쇄로 버그가 드러났는데,
전부 "1배속에선 프레임당 변화량이 작아서 안 보이던 근사 오차가, 배속이 커지면서 눈에 보일
만큼 커진" 같은 패턴이었다.

## 2. 20배속·100배속 버튼을 눌러도 조용히 10배속으로 잘림

**증상**
배속 버튼을 20이나 100으로 눌러도 체감 속도가 10배속과 똑같음.

**원인**
```cpp
// GameInstance.h
void SetTimeScale(float scale) { _timeScale = clamp(scale, 0.f, 10.f); }
```
배속 기능을 처음 설계할 때 잡아둔 상한(10)이 그대로 남아있어서, 그보다 큰 값은 전부
10으로 잘렸다.

**해결**
```cpp
void SetTimeScale(float scale) { _timeScale = clamp(scale, 0.f, 100.f); }
```

## 3. 배속을 걸면 행성은 빨라지는데 우주선은 거의 안 움직임

**증상**
10배속만 걸어도 행성들은 쌩쌩 도는데 우주선만 제자리에 가깝게 멈춰있는 것처럼 보임.

**원인**
행성은 `_meanAnomaly += _orbitSpeed * deltaTime`으로 그 프레임의 deltaTime을 상한 없이
그대로 반영하는데, 우주선의 `UPhysicsComponent`는 고정 타임스텝 누산기 방식이라 사정이
다르다.
```cpp
// UPhysicsComponent::Update()
_accumulator += deltaTime;
while (_accumulator >= FIXED_DT && steps < MAX_STEPS)   // FIXED_DT = 1/240초
{
    PhysicsStep(FIXED_DT);
    ...
}
if (steps >= MAX_STEPS) _accumulator = 0.f;   // 밀린 시간은 버린다
```
`MAX_STEPS = 25`는 원래 "배속 없이 프레임이 심하게 끊겨도(`GMaxDeltaTime = 0.1초`) 그
한 프레임 분량은 다 소화할 수 있게"(`25 × 1/240 ≈ 0.104초`) 튜닝된 값인데, 배속이
도입되면서 아무도 이 상수를 다시 안 건드렸다. 100배속이면 한 프레임에 최대
`0.1초 × 100 = 10초`치 게임 시간이 들어올 수 있는데, 필요한 스텝 수는
`10 / (1/240) = 2400`개 — `MAX_STEPS=25`론 어림도 없이 잘려서 나머지 대부분이 그냥
버려진다.

**해결**
```cpp
static constexpr int MAX_STEPS = 3000;   // 25 → 3000
```
스텝 크기(`FIXED_DT`)는 그대로 유지해서 적분 정확도·안정성은 배속과 무관하게 동일하게
유지하고, "같은 크기 스텝을 몇 번 더 도느냐"만 늘렸다. 스텝 하나의 연산이 가벼워서(우주선
1기, 액터 수도 적음) 3000번을 돌아도 성능 영향은 사실상 없다.

## 4. MAX_STEPS를 고쳤더니, 이번엔 배속을 걸수록 점점 궤도를 못 따라감

**증상**
3번을 고친 뒤에도, 10배속으로 한동안 두면 우주선이 서서히 목표 궤도에서 벗어남 — 즉시
깨지는 게 아니라 "점점" 벌어짐.

**원인**
`MainWorld::Update()`는 행성(우주선의 중력원 포함)을 먼저 갱신하고 나서 우주선의 물리
서브스텝 루프를 돈다. 배속이 크면 그 프레임의 deltaTime도 커서, **행성이 먼저 그 큰
deltaTime만큼 한 번에 "미래" 위치로 점프**해버린 뒤에야 우주선의 서브스텝(많으면 수백~
수천 번)이 도는데, 그 모든 서브스텝이 전부 "이미 점프한 행성의 미래 위치" 하나만 보고
중력 방향을 계산한다. 1배속에선 프레임당 점프량이 작아서 안 보이던 이 오차가, 배속이
커질수록 매 프레임 같은 방향으로 계속 쌓인다.

**해결**
`APlanet::GetFuturePos(t)`(닫힌 형태로 t초 뒤 위치를 즉시 계산하는 함수, 이미 존재)를
활용해서, 우주선의 서브스텝마다 "그 스텝이 실제로 놓인 시점의 행성 위치"를 정확히 물어보게
바꿨다.
```cpp
// UPhysicsComponent::Update()
_accumulator += deltaTime;
float timeOffset = -_accumulator;   // 처리 대기 중인 물리 큐의 시작점(과거)
int steps = 0;
while (_accumulator >= FIXED_DT && steps < MAX_STEPS)
{
    PhysicsStep(FIXED_DT, timeOffset);
    timeOffset += FIXED_DT;         // 스텝마다 '현재'에 한 스텝씩 가까워짐
    _accumulator -= FIXED_DT;
    ++steps;
}

// ComputeAcceleration()의 targetPlanet 중력 부분
Vector2 targetPos = targetPlanet->GetFuturePos(targetTimeOffset);   // GetCenterPos() 대신
Vector2 dir = targetPos - pos;
```
`targetPlanet->GetAcceleration()`(프레임 보정항)은 그대로 뒀다 — 가속도는 위치와 달리
스텝 사이 변화가 미미해서, 이 정도 근사는 무시할 수준이었다.

> 📌 **참고 (버그 아님):** 위 수정 이후에도 고배속에서 궤도 모양/방향이 계속 도는 것처럼
> 보이는 현상이 남아있는데, 이건 실제 물리다. `ComputeAcceleration()`의
> `a += targetPlanet->GetAcceleration()` 항 때문에, 우주선은 지구가 태양을 돌며 겪는
> 가속도 방향 변화를 그대로 같이 받는다 — 실제 인공위성도 지구가 태양을 도는 동안
> 궤도면이 서서히 세차운동을 한다. 지구 공전주기(게임 시간으로 약 314초)만큼이 1배속에선
> 안 보일 만큼 느리지만, 100배속에선 3초 안팎으로 압축되어 눈에 보이게 된 것뿐이다.
> 탈출하지 않고 모양만 계속 바뀌는 것도 이 설명과 정확히 들어맞아서, 굳이 감쇠시키지 않고
> 그대로 두기로 했다(정직한 물리 vs 고배속에서 깔끔해 보이는 연출, 둘 중 정직한 쪽 선택).

---

# Part 3. 호만전이 × 고배속

## 5. 고배속으로 행성 근처를 스쳐 지나가면 도착(포획) 판정을 통째로 놓침

**증상**
배속을 걸고 호만전이를 실행하면, 우주선이 목적지 행성을 그냥 지나쳐서 계속 태양 기준
궤도로 날아감. "버튼 눌렀을 때의 위치로 날아가서 멈춘 것 같다"는 인상.

**원인**
도착 판정이 "이번 프레임이 끝난 시점의 점 하나"만 보고 SOI(영향권) 반경 안인지 검사한다.
```cpp
Vector2 toShip = _ship->GetCenterPos() - _transferTarget->GetCenterPos();
if (toShip.LengthSquared() <= soi * soi) { /* 포획 */ }
```
1배속에선 우주선이 한 프레임에 이동하는 거리가 SOI 반경보다 훨씬 작아서 항상 어느
프레임에서든 걸리는데, 배속이 크면 한 프레임 이동 거리가 SOI 지름보다 커질 수 있다 —
그러면 행성 옆을 "휙" 스쳐 지나가면서 SOI 안에 있던 순간 자체가 어느 프레임에도 찍히지
않고 통째로 건너뛰어진다(터널링).

**해결**
"이번 프레임 끝난 점" 대신 "이번 프레임에 지나온 경로(직선 구간) 전체" 중 행성과 가장
가까웠던 지점으로 검사하도록 바꿨다.
```cpp
Vector2 shipPosBeforeUpdate = _ship->GetCenterPos();   // 물리 갱신 전 위치, 루프 밖에서 저장
Super::Update(deltaTime);
...
Vector2 seg = _ship->GetCenterPos() - shipPosBeforeUpdate;
float segLenSq = seg.LengthSquared();
float t = (segLenSq > 0.0001f)
    ? clamp((planetPos - shipPosBeforeUpdate).Dot(seg) / segLenSq, 0.f, 1.f) : 0.f;
Vector2 closestOnPath = shipPosBeforeUpdate + seg * t;

if ((closestOnPath - planetPos).LengthSquared() <= soi * soi)
{
    // 포획 성립 시 실제 스냅 위치/속도는 여전히 "현재(프레임 끝)" 기준 라이브 데이터를 그대로 사용
    ...
}
```
얼마나 빨리 움직이든 절대 못 건너뛰게 됐고, 실제로 궤도에 스냅되는 위치/속도 계산 자체는
안 바뀌었다(검사 방식만 점 → 선분).

## 6. 배속으로 오래 플레이한 뒤 전이를 걸면 목적지와 도착 위치가 크게 어긋남

**증상**
배속을 걸고 한참 지난 뒤 호만전이를 실행하면, 계산된 궤적이 실제 목적지 행성 위치와 전혀
다른 곳을 겨냥함.

**원인**
`SolveLambert(r1, r2, tof, mu)`(Universal Variable 방식의 고전적인 람베르트 솔버)는 **초점
(태양)이 원점 `(0,0)`에 있다고 가정**하고 `r1`/`r2`의 길이·내적·외적만으로 계산한다.
태양이 고정이던 예전엔 절대좌표를 그대로 넘겨도 우연히 맞았는데, Part 1에서 태양을 실제
은하 궤도로 돌게 만들면서 `_sun->GetCenterPos()`가 원점에서 서서히 멀어지기 시작했다 —
그런데 `FindBestTransfer()`/`ScanTOFRange()`/`ComputePhaseAngleError()`는 여전히
`shipPos`/`target->GetCenterPos()` **절대좌표를 그대로** 람베르트에 넘기고 있었다.

배속으로 게임 시간이 실제 시간보다 수십~수백 배 빨리 흐르니, 태양의 은하 드리프트가
누적되어 원점에서 수만 유닛 단위로 벌어질 수 있다(금성 궤도 반지름 자체가 40,000).
그러면 "실제 태양이 아니라 원점에 있는 유령 태양" 기준으로 계산된 궤적이 완전히 딴 곳을
겨냥하게 된다. "배속 상태에서만" 티가 나는 이유가 정확히 이거다 — 짧게 플레이하면 드리프트가
작아서 안 보인다.

**해결**
태양의 실제 위치(`sunPos`)를 인자로 받아서, 모든 위치를 태양 기준 상대좌표로 바꾼 뒤에
람베르트에 넘기도록 시그니처를 확장했다.
```cpp
// InterplanetaryTransfer.h
TransferPlan FindBestTransfer(Vector2 shipPos, Vector2 shipVel, APlanet* target,
                               Vector2 sunPos, Vector2 sunVel, float sunMu);

// ScanTOFRange() 내부
Vector2 r1 = shipPos - sunPos;
Vector2 r2 = target->GetFuturePos(tof) - sunPos;
LambertResult lam = SolveLambert(r1, r2, tof, sunMu);

// 람베르트 결과는 "태양이 원점인 관성계" 기준 속도라, 태양 자신의 절대속도를 더해야
// 물리엔진이 쓰는 월드(절대) 속도가 된다.
Vector2 v1Abs = lam.v1 + sunVel;
Vector2 v2Abs = lam.v2 + sunVel;
```
`PredictTransferPath()`(궤적 프리뷰용 RK4 전파 함수)는 원래부터 `sunPos`를 인자로 받아
절대좌표로 계산하고 있어서 이 버그가 없었다 — 문제는 딱 람베르트 솔버를 감싸는 쪽에만
있었다.

> 📌 같은 "태양=원점 가정" 패턴이 `MainWorld::GetReachablePlanets()`(호만전이 후보 필터링)와
> `OnSceneGUI()`(ImGui 디버그 텍스트)에도 남아있었다. 전자는 게임플레이에 영향이 있어서
> 같이 고쳤고, 후자는 디버그 표시용이라 우선순위를 낮게 잡아 보류했다(GUI를 자주 안 볼
> 예정이라는 판단).

---

# Part 4. 위성(달) 추가

## 기능 구현 요약

기획서/로드맵에는 "달 전이·착륙·귀환"이 스코프 폭발로 분류되어 "확장 아이디어로 기록만"
해뒀던 항목인데, 이번에 시각적인 위성(공전 + 그림자 + 궤도선)까지는 부담 없이 구현할 수
있다는 게 Part 1의 `_orbitCenterBody` 체인 덕분에 드러나서 실제로 만들었다.

- **클래스 재사용**: 새 클래스 없이 `APlanet`을 그대로 씀 — "달이 지구를 돈다"는
  "지구가 태양을 돈다"와 구조적으로 동일한 문제라, `_orbitCenterBody = earth`로 연결하는
  것만으로 끝난다.
- **`IsMoon()` 플래그**: 위성을 SOI 자동 타겟팅(`ASpaceship::UpdateTargetPlanet()`)과
  호만전이 대상, 액터 버튼 목록에서 명시적으로 제외했다. 지금 시스템은 "행성은 전부 태양
  1단계 자식"이라는 가정만 알고 있어서, 위성처럼 "행성을 도는 행성"까지 SOI/전이가
  다루려면 계층적 스캔으로 다시 짜야 하는 큰 작업이 필요하다 — 지금은 시각적 요소로만
  스코프를 잘랐다.
- **UI**: 부모 행성(지구) 버튼과 같은 행, 오른쪽에 별도 버튼을 붙이는 `ShowMoonButtons()`를
  `Widget_Main`에 추가. 앞으로 추가될 위성(포보스·데이모스, 갈릴레이 위성 등)도 같은 구조로
  재사용하도록 `kMaxMoonButtons=16`을 미리 잡아뒀다.
- **달 궤도 파라미터**: 실제 축척(지구 반지름 대비 약 60배)을 그대로 쓰면 지구 SOI(약
  6000) 밖으로 나가버려서, 지구 SOI 안에 넉넉히 들어오는 반지름 2800, 지구 공전보다 훨씬
  빠른 각속도(0.26rad/s, 24초/바퀴)로 눈에 띄게 스케일을 조정했다. (이 프로젝트의 다른
  행성 간 거리도 애초에 실제 축척이 아니라 게임 스케일이라, 일관된 선택.)

## 7. 달이 지구가 아니라 태양 주변을 도는 것처럼 보임

**증상**
달을 추가하고 실행하니, 지구가 아니라 태양 주변의 작은 궤도를 도는 것처럼 보임.

**원인**
Part 1에서 만든 "모든 행성을 태양에 연결"하는 루프(`InitPlanet()` 끝, 매 프레임 갱신
루프와 별개로 초기화 시점에도 한 번 도는 코드)가 `_planets` 리스트에 있는 항목을 태양이
아니면 전부 태양에 연결한다.
```cpp
for (auto planet : _planets)
{
    if (planet == _sun) continue;
    planet->SetOrbitCenter(_sun->GetCenterPos(), _sun);   // 위성도 여기 걸림
}
```
달을 `_planets`에 넣으면서(궤도선·그림자 렌더링 루프가 이 리스트를 쓰기 때문에 필요함)
이 루프도 같이 걸려버렸다 — 지구에 연결해뒀던 `_orbitCenterBody`를 태양으로
덮어써서, 달이 자기 궤도 반지름(2800)으로 **태양 바로 옆**을 도는 모양이 됐다.

**해결**
```cpp
for (auto planet : _planets)
{
    if (planet == _sun) continue;
    if (planet->IsMoon()) continue;   // 위성은 이미 자기 행성에 연결돼있음 — 덮어쓰면 안 됨
    planet->SetOrbitCenter(_sun->GetCenterPos(), _sun);
}
```
매 프레임 갱신 루프(`RefreshOrbitCenter()`)는 각자 자기 `_orbitCenterBody`를 그대로
참조하는 구조라 원래부터 문제 없었고, 문제는 초기화 시점의 이 한 번짜리 루프뿐이었다.

---

## 이후 과제 (보류)

- **착륙**: 판정(`OrbitalGameMode`의 기존 크래시 감지 재사용 + 상대속도 임계값)과 "행성에
  붙어서 같이 이동"(착륙 = 공전속도 0인 위성과 동일한 문제, `_orbitCenterBody` 패턴 재사용
  가능) 둘 다 기술적으로는 크지 않지만, 이번엔 스코프에서 제외.
- **위성의 SOI/전이 지원**: 지금은 시각 전용. 화성 위성(포보스·데이모스)부터 목성 갈릴레이
  위성까지 계획대로 늘리려면, `UpdateTargetPlanet()`/호만전이 스캔을 "태양 → 행성 → 위성"
  계층 구조로 다시 짜는 작업이 언젠가 필요하다.
- **사이드뷰(`V`키)에서 태양계 공전궤도를 원형으로 보고 싶다는 요청** — 3주차 이후 어느
  시점에 나왔다가 답을 못 받고 넘어간 채로 남아있음. 여전히 유효하면 다시 논의 필요.
