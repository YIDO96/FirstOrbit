# 2주차 트러블슈팅

> 이번 주 작업 중 실제로 막혔다가 해결한 것들을 증상 → 원인 → 해결 순으로 남긴다.
> 3주차 기술 회고 문서 쓸 때 재료로 그대로 쓸 수 있게, 코드 위치(파일:줄)를 같이 적어둔다.

---

# Part 1. Day 4 (적분기 + 고정 타임스텝)

## 1. `static` 비-`const` 멤버를 정의 없이 선언해서 링크 에러

**증상**
```
UPhysicsComponent.obj : error LNK2001: 확인할 수 없는 외부 기호 "private: static float UPhysicsComponent::FIXED_DT"
UPhysicsComponent.obj : error LNK2001: 확인할 수 없는 외부 기호 "private: static int UPhysicsComponent::MAX_STEPS"
FirstOrbit.exe : fatal error LNK1120: 2개의 확인할 수 없는 외부 참조입니다.
```

**원인**
고정 타임스텝 도입하면서 헤더에 이렇게 선언했음:
```cpp
static float FIXED_DT;
static int MAX_STEPS;
```
`const`/`constexpr`가 아닌 일반 `static` 멤버는 클래스 안의 선언이 **선언일 뿐**이고, 실제
저장공간은 어딘가의 `.cpp`에서 따로 정의해야 한다. `.cpp`에 그 정의가 없어서 링커가
심볼을 못 찾은 것.

**해결**
`ASpaceship.h`에 이미 있던 관례(`static constexpr int32 maxTrail = 1500;`)를 그대로 따라
`static constexpr`로 바꿔서 클래스 안에서 바로 정의되게 함 — `.cpp` 쪽 정의가 필요 없어짐.
```cpp
// UPhysicsComponent.h
static constexpr float FIXED_DT = 1.f / 240.f;
static constexpr int MAX_STEPS = 5;
```
> 📌 값을 나중에 ImGui 슬라이더로 런타임에 바꾸고 싶어지면, 그때는 `constexpr`을 빼고
> 인스턴스 멤버로 바꾸면 된다. 지금은 상수로 충분.

---

# Part 2. Day 5 (발사 시퀀스 상태 머신)

## 2. `TimeManager`의 non-loop 타이머가 발동 후에도 스스로 안 없어짐

**증상 (코드 리뷰 중 발견 — 실제 버그로 터지기 전에 잡음)**
카운트다운(`TIME.AddTimer(func, 3.f)`, `loop=false`)을 쓰려고 `TimeManager.cpp`를 읽어보니,
`Timer::Update()`가 만료 시점에 콜백만 호출하고 자기 자신을 제거 요청하는 코드가 없었음.

**원인**
```cpp
void Timer::Update(float deltaTime)
{
    _sumTime += deltaTime;
    if (_sumTime >= _interval)
    {
        _func();
        if (_loop) { _sumTime -= _interval; }
        // ← loop가 아니면 여기서 끝. _sumTime은 그대로 _interval 이상으로 남아있음
    }
}
```
`loop=false`인 타이머는 한 번 발동한 뒤에도 `_sumTime`이 계속 `_interval` 이상인 상태로
남아서 **다음 프레임부터 매 프레임 콜백이 다시 호출**된다. `Timer::IsExpired()`가 이미
선언돼 있었지만 `TimeManager.cpp` 어디서도 호출되지 않는 걸로 봐서, 만료 후 자동 제거를
만들려다 마무리를 안 한 것으로 보임. 이 프로젝트에서 `AddTimer`를 실제로 쓴 게
카운트다운이 처음이라 아무도 안 겪어본 함정이었음.

**해결**
매번 호출하는 쪽이 `TIME.Remove(id)`를 직접 챙기게 하는 대신, `TimeManager` 쪽에서
근본적으로 고침 — `Timer::Update()`가 "제거해야 하는가"를 반환하도록 시그니처 변경.
```cpp
// TimeManager.h
bool Update(float deltaTime);   // void → bool

// TimeManager.cpp
bool Timer::Update(float deltaTime)
{
    _sumTime += deltaTime;
    if (_sumTime >= _interval)
    {
        _func();
        if (_loop) { _sumTime -= _interval; return false; }
        return true;   // 한 번 쏘고 끝 — 제거해달라는 신호
    }
    return false;
}

// TimeManager::Update()
for (auto& iter : _timers)
{
    if (iter.Update(_deltaTime))
        _removeTimers.insert(iter.GetId());
}
```
> 📌 **엔진 레벨에서 고친 이유:** 호출하는 쪽마다 "타이머 다 쓰면 직접 지워야 함"을
> 기억하게 하는 것보다, 근본 원인(엔진)을 고치면 **앞으로 이 프로젝트에서 `AddTimer`를
> 쓰는 모든 곳**이 자동으로 혜택을 받는다. 기존 삭제 지연 패턴(`_removeTimers`를 다음
> 프레임 맨 앞에서 처리)과도 안전하게 맞물림 — 순회 중에 `set`에 넣는 것뿐이라 반복자
> 무효화가 없음.

---

## 3. 회전이 갑자기 추력(`W`)에 종속돼버림 (회귀)

**증상**
"발사 대기 중엔 `A`/`D`가 안 먹혔으면 좋겠다"는 의도로 `Input()`을 고쳤는데, 이후 `W`를
같이 안 누르면 **비행 중에도 회전 자체가 안 먹히게** 됨.

**원인**
```cpp
void ASpaceship::Input(float deltaTime)
{
    _isThrusting = _INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up);
    if (not _isThrusting) return;   // ← 이 한 줄 때문에 회전도 같이 막힘

    if (...A/D...) AddRotation(...);
}
```
막고 싶었던 건 "발사 대기 상태"인데, 실제로 쓴 조건은 "이번 프레임에 추력을 켰는가"였다.
전혀 다른 조건이라, 비행 중에 엔진을 끄고(관성으로 날아가면서) 자세만 미리 돌려두는
정상적인 조작(현실의 RCS처럼 자세제어와 메인엔진을 분리해서 쓰는 것)까지 막혀버림.

**해결**
막고 싶은 건 "지금 조작 가능한 상태인가"이므로, 이미 있는 `UPhysicsComponent::_isPaused`를
그대로 재사용. 물리가 멈춰있으면(=`Idle`/`Countdown`) 입력 자체를 안 받고, 물리가 돌면(=
`Ascent`) 추력 여부와 무관하게 회전은 항상 가능.
```cpp
void ASpaceship::Input(float deltaTime)
{
    if (_physicsComp->GetIsPaused()) return;   // 발사 대기 중엔 조작 자체를 막는다

    if (...A/D...) AddRotation(...);
    _isThrusting = _INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up);
}
```
> 📌 **교훈:** "막고 싶은 조건"을 코드로 옮길 때, 손에 잡히는 아무 플래그(`_isThrusting`)로
> 대체하지 말고 **실제로 뜻하는 조건**(지금 여기선 "물리가 멈춰있는가")이 이미 있는지부터
> 찾을 것. 이미 있는 `_isPaused`를 썼기 때문에 `LaunchGameMode`가 상태를 바꿀 때마다
> 자동으로 같이 갱신되고, 코드가 하나 더 안 늘었다.

---

# Part 3. Day 6 (LaunchWorld → MainWorld 전환)

## 4. `MainWorld`의 우주선이 중력도 추력도 전혀 안 먹힘

**증상**
`MainWorld`에서 우주선을 가만히 둬도 안 떨어지고, `KEY_0`으로 원궤도 속도를 넣어도 제자리에
그대로 멈춰있음.

**원인**
Day 5에서 `UPhysicsComponent`에 `_isPaused` 플래그를 추가하면서 기본값을 `true`로 뒀다
(발사 대기 중 우주선이 멈춰있어야 하는 `LaunchWorld` 기준). 그런데 `MainWorld::Enter()`엔
`SetPaused(false)`를 호출하는 코드가 어디에도 없어서, `UPhysicsComponent::Update()`가
매 프레임 그냥 리턴만 하고 있었음.
```cpp
void UPhysicsComponent::Update(float deltaTime)
{
    if (_isPaused) return;   // MainWorld 우주선은 여기서 항상 멈춤
    ...
}
```

**해결**
`MainWorld`는 카운트다운으로 정지시켰다 풀 필요가 없으므로, 스폰 직후 한 번만 풀어주면 됨.
```cpp
// MainWorld::Enter()
_ship->GetComponent<UPhysicsComponent>()->SetPaused(false);
```
> 📌 기본값을 `false`로 바꾸는 대신 각 `World`가 스폰 시점에 명시적으로 설정하게 둔 이유:
> `LaunchWorld`는 여전히 "기본은 정지"가 맞는 동작이라, 컴포넌트의 기본값 자체를 바꾸면
> 그쪽이 다시 깨진다. `SetTargetPlanet()`처럼 월드별 스폰 설정은 원래도 `Enter()`에서
> 개별적으로 해주던 것들이라 패턴이 일관적이다.

---

# Part 4. 빌드 환경 (주차 내내 반복)

## 5. `C1853`(PCH 불일치) / `LNK2005`(이미 정의됨) — 증분 빌드가 간헐적으로 깨짐

**증상**
```
error C1853: 'x64\Debug\FirstOrbit.pch' 미리 컴파일된 헤더 파일이 다른 버전의 컴파일러에서
만들어졌거나 ...
```
그리고 별개로
```
"...stateName" (...)이(가) LaunchGameMode.obj에 이미 정의되어 있습니다.
```
`stateName`은 `LaunchGameMode`의 **private 인스턴스 멤버**라 원래 이런 에러가 날 이유가
없는 코드인데도 발생.

**원인**
정확한 근본 원인은 못 밝혔지만, 재현 패턴은 확인됨: **일반(증분) 빌드에서 재현되고,
`/t:Rebuild`(전체 다시 빌드)로는 매번 100% 사라진다.** 즉 소스 코드 문제가 아니라
증분 빌드의 의존성 추적(타임스탬프/오브젝트 캐시)이 이 프로젝트 환경에서 간헐적으로
어긋나는 문제로 보임.

**해결**
증상이 뜨면 Visual Studio에서 **빌드 → 솔루션 다시 빌드**(일반 빌드보다 느리지만 확실함)를
사용. 매번 전체 리빌드하기 번거로우면, 증상이 재현될 때만 그렇게 대응.

---

## 6. AI(Claude)의 커맨드라인 빌드와 IDE 빌드가 서로 충돌

**증상**
Claude가 확인 차 커맨드라인으로 `MSBuild`를 돌릴 때마다, 그 이후 Visual Studio에서
빌드하면 위 5번(`C1853`) 증상이 뜸.

**원인**
커맨드라인 빌드와 IDE 빌드가 **같은 중간 산출물 디렉토리**(`x64/Debug`)를 공유하는데,
서로 다른 두 빌드 프로세스가 번갈아 건드리면서 증분 빌드 캐시가 더 쉽게 어긋남.

**해결**
Claude가 검증용으로 직접 빌드하는 걸 중단하고, 코드를 수정한 뒤엔 **사용자가 직접
Visual Studio에서 빌드해서 결과(성공/에러 메시지)를 알려주는 방식**으로 워크플로우를 바꿈.

> 📌 AI 어시스턴트에게 "확인해달라"고 할 때, 그 확인 방법(예: 커맨드라인 빌드)이 실제
> 개발 환경(IDE)과 충돌할 수 있다는 걸 겪은 사례. 도구가 같은 자원을 공유하면 "검증"
> 자체가 새로운 문제를 만들 수 있다.

---

# Part 5. (참고) 아직 미해결 — 3주차로 이월

## 7. ⚠️ `MainWorld` 핸드오프 위치 계산에서 표면 오프셋 누락 (미해결)

**증상 (아직 실제로 안 터짐 — 코드 재검토 중 발견)**
`MainWorld::Enter()`에서 `LaunchHandoff`를 소비하는 코드:
```cpp
Vector2 launchOrigin = _homePlanet->GetCenterPos() + Vector2(0.f, -_homePlanet->GetBodyRadius());
_ship->SetCenterPos(_homePlanet->GetCenterPos() + handoff.position);   // ← launchOrigin을 안 씀
```
`launchOrigin`을 계산만 해두고 실제 위치 대입에는 안 쓰고 있음.

**원인 (추정)**
원래 의도는 "발사대 = 홈 행성 꼭대기(표면)"이므로 `launchOrigin`(표면 지점) 기준으로
`handoff.position`(LaunchWorld 로컬 좌표)을 더해야 하는데, 리팩토링 중 `launchOrigin` 대신
행성 **중심**에 바로 더하는 줄만 남았다.

**영향**
`LaunchWorld`에서 고도 1000 지점에 도달해 전환하므로 `handoff.position.y ≈ -1000`.
`bodyRadius`도 1000이라, 지금 코드로는 도착 시 행성 중심으로부터의 거리가
**대략 `bodyRadius`와 같거나 아주 조금 큰 값**이 된다 — 즉 도착하자마자 지표면에
거의 붙어있는 상태. 3주차 Day 3에서 추락 판정(`r < bodyRadius`)을 만들면 도착 직후
바로 실패로 뜰 가능성이 높다.

**해결 (예정)**
```cpp
_ship->SetCenterPos(launchOrigin + handoff.position);   // 표면 기준으로 더하기
```
3주차 Day 3(승리/실패 판정) 들어가기 전에 먼저 고칠 것.
