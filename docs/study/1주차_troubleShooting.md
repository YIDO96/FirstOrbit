# 1주차 트러블슈팅

> 이번 주 작업 중 실제로 막혔다가 해결한 것들을 증상 → 원인 → 해결 순으로 남긴다.
> 3주차 기술 회고 문서 쓸 때 재료로 그대로 쓸 수 있게, 코드 위치(파일:줄)를 같이 적어둔다.

---

# Part 1. Day 4 (B-6 마무리 + B-7 행성)

## 1. `C1853`: 미리 컴파일된 헤더 오류

**증상**
```
error C1853: 'x64\Debug\FirstOrbit.pch' 미리 컴파일된 헤더 파일이 다른 버전의
컴파일러에서 만들어졌거나, 미리 컴파일된 헤더가 C++인데 C에서 사용하고 있거나
그 반대의 경우입니다.
```
`GameInstance.cpp`, `Camera.cpp`, `Texture.cpp`, `MainWorld.cpp` 등 여러 소스에서 동시에 발생.

**원인**
소스 코드 문제가 아니라 **빌드 캐시 문제**. `x64/Debug/FirstOrbit.pch`(234MB)가 지금 설치된 컴파일러(VS2022, MSVC 14.44) 버전과 안 맞는 상태로 남아있었음 — 다른 컴퓨터에서 빌드했거나, 컴파일러/VS가 그 사이 업데이트된 경우 발생.

**해결**
`.gitignore`로 이미 제외된 빌드 산출물이라 안전하게 삭제 후 재빌드하면 자동 재생성됨.
```
x64/Debug/FirstOrbit.pch
x64/Debug/vc143.idb
x64/Debug/vc143.pdb
```

---

## 2. ImGui 창 위에서 카메라 드래그·줌이 같이 동작함

**증상**
슬라이더 등 ImGui 창을 클릭+드래그하면 그 마우스 델타가 그대로 카메라 이동/줌에도 반영됨.

**원인**
`Camera::UpdateZoom()`에는 `io.WantCaptureMouse` 체크가 있었는데, `Camera::UpdateDrag()`에는 같은 체크가 없었음. 개별 함수마다 따로 가드를 두다 보니 하나가 빠진 것.

**해결**
가드를 한 곳(`Camera::Update()`)으로 모아서 뷰포트 밖 판정과 함께 처리.
```cpp
// Camera.cpp
ImGuiIO& io = ImGui::GetIO();
if (!_INPUT.IsMouseInsideWindow(GAME.GetGameViewportRect()) or io.WantCaptureMouse)
{
    _targetZoom = _zoom;
    UpdateViewMatrix();
    return;
}
```
개별 함수마다 체크하는 대신 상위에서 한 번에 걸러내니 `UpdateZoom`/`UpdateDrag` 둘 다 동일하게 보호됨.

---

## 3. `World` 베이스에 `GetCamera()`가 없어서 `APlanet::Render()`가 컴파일 안 됨

**증상**
`APlanet::Render()`에서 `_ownerWorld->GetCamera()`를 호출하려는데, `_ownerWorld`가 `World*` 타입이라 컴파일 에러.

**원인**
카메라가 `World` 베이스가 아니라 `MainWorld` 전용 멤버로 있었음 (`MainWorld::_camera`, `MainWorld::GetCamera()`). `AActor::_ownerWorld`는 항상 `World*`이므로 파생 클래스 전용 멤버엔 접근할 수 없음.

**해결**
Day 1 원래 계획(`1-4`)대로 `_camera`를 `World` 베이스로 승격.
```cpp
// World.h
public:    Camera& GetCamera() { return _camera; }
protected: Camera  _camera;
```
`World::Enter()`/`World::Update()`에서 `_camera.Init()`/`Update()`를 호출하도록 옮기고, `MainWorld`의 중복 멤버는 제거.

---

## 4. 콜라이더를 붙였는데 `AActor::GetCollider()`가 항상 `nullptr`

**증상**
`APlanet::Init()`에서 `AddComponent<UCircleColliderComponent>()`로 컴포넌트를 만들었는데, `World`의 브로드페이즈나 `GetCollider()`가 이 컴포넌트를 전혀 인식 못 함.

**원인**
```cpp
class UCircleColliderComponent* collider;   // APlanet 자체의 별도 필드
collider = AddComponent<UCircleColliderComponent>();
```
`AActor`엔 이미 protected `_collider`(브로드페이즈/`GetCollider()`가 실제로 참조하는 필드)가 있는데, 여기다 저장하지 않고 이름만 비슷한 **자기 자신의 별도 변수**에 저장해서 두 필드가 따로 놀았음.

**해결**
구체 타입 포인터(`SetRadius()` 호출용)와 base 포인터(`GetCollider()`용)를 **둘 다** 채움 — 둘은 같은 객체를 가리키는 포인터일 뿐이라 순서 상관없이 최신 상태를 공유함.
```cpp
_circleCollider = AddComponent<UCircleColliderComponent>();
_collider = _circleCollider;   // AActor의 protected _collider에도 등록
```

---

## 5. `Render()`에 LOD 판정만 있고 실제로 그리는 코드가 없어서 화면에 아무것도 안 보임

**증상**
`WorldToScreenScale()`로 화면상 반지름(`r`)까지 계산은 되는데, `if/else` 분기 안이 비어있어서 행성이 렌더링되지 않음.

**원인**
LOD **판정 로직**(어느 단계로 그릴지 정하는 것)과 **실제로 그리는 코드**(GDI 호출)를 착각해서, 판정만 만들고 그리는 부분을 안 채운 채로 넘어감.

**해결**
처음엔 `Ellipse`/`SetPixel`로 3단계(원+테두리 / 단색 원 / 점)를 채웠다가, 이후 텍스처를 적용하면서 `Texture::Render()`의 `destSize` 인자로 화면 크기(`r`)에 맞춰 **연속적으로 스케일링**하는 방식으로 교체함. 결과적으로 멀어지면 자연히 1px까지 작아지므로, 원래 계획한 이산적 3단계와 시각적 결과는 동등함.

> 📌 설계 변경 메모: 원래 로드맵은 `r≥30`/`r≥3`/`else` 3단계 이산 분기를 제안했지만, 텍스처 기반으로 가면서 연속 스케일링으로 대체함. "왜 이산 LOD 대신 연속 스케일로 갔는가"는 회고에 적어둘 만한 판단 근거.

---

## 6. 초기 카메라 줌(1.0)이 태양계 스케일과 안 맞아서 첫 실행 시 아무것도 안 보임

**증상**
태양 반지름 `5000`~`12000`, 지구 공전반지름 `60000` 같은 스케일인데 카메라 `_zoom` 기본값이 `1.f`라, 실행하면 카메라가 태양 표면 속에 파묻힌 것처럼 보임.

**원인**
1주차 시작 파라미터 표는 "800px 화면" 가정으로 계산된 값인데, 실제 `GWinSizeX/Y`는 `480x800`(세로형)이라 스케일 감이 다름. 줌 보정 없이 기본값 그대로면 태양계 전체가 화면 밖.

**해결**
`MainWorld::Enter()`에서 `_camera.SetZoomImmediate(0.01f)`로 시작 줌을 미리 잡아둠.

---

## 7. 클릭 피킹 — 좌표는 저장되는데 실제 판정 로직이 없어서 클릭이 반응 없음

**증상**
`Camera`가 클릭 시점의 월드 좌표(`_mouseWorldPos`)는 저장하고 있었지만, 그 좌표가 어떤 행성 위인지 비교하는 코드가 어디에도 없어서 클릭해도 아무 일도 안 일어남.

**원인**
B-6 단계에서 좌표 저장까지만 해두고, 실제 대상(행성)이 아직 없어서 판정 로직을 미뤄뒀던 것 — B-7에서 `APlanet`이 생긴 뒤에도 이어붙이는 걸 깜빡함.

**해결**
`World`에 범용 히트테스트 함수를 추가해서, 이미 연결해둔 콜라이더(항목 4)를 재사용:
```cpp
// World.cpp
AActor* World::PickActor(Vector2 worldPos) const
{
    for (AActor* actor : _actors)
    {
        UColliderComponent* col = actor->GetCollider();
        if (!col || col->GetColliderType() != ColliderType::Circle) continue;
        if ((worldPos - actor->GetCenterPos()).Length() <= col->GetRadius())
            return actor;
    }
    return nullptr;
}
```
`MainWorld::Update()`에서 클릭 시 `WorldToMousePos()` → `PickActor()` 순으로 호출.

> 📌 이 `PickActor()`는 B-9(카메라 모드 전환)에서 "클릭한 액터를 팔로우 타겟으로 삼는" 기능에 그대로 재사용될 예정.

---

## 8. (참고) 설계 문서가 실제 코드보다 낡아있던 사례

`구현_설계_미리보기.md`는 "`Texture::Render()`엔 배율 인자가 없어 스프라이트를 줌에 맞춰 확대/축소할 수 없다"고 적혀있었는데, 실제로는 `Texture::Render()`가 이미 `destSize` 인자를 받아 `StretchBlt`/`TransparentBlt`로 스케일링을 지원하고 있었음(문서 작성 이후 코드가 업데이트된 것으로 보임). **문서를 그대로 믿지 말고 실제 헤더/구현을 확인하는 습관**이 중요하다는 걸 보여준 사례.

---
---

# Part 2. Day 5~7 (설계 전환 + 카메라 보간 + 변환 합성)

## 9. `MSB8020`: 플랫폼 도구 집합 v145를 찾을 수 없음

**증상**
```
error MSB8020: v145에 대한 빌드 도구(플랫폼 도구 집합 = 'v145')를 찾을 수 없습니다.
```
잘 되던 빌드가 갑자기 컴파일 시작조차 못 함.

**원인**
`.vcxproj`의 `PlatformToolset`이 `v143` → `v145`로 바뀌어 있었음. VS를 다시 열었을 때 뜬 **"솔루션 대상 변경(Retarget)"** 안내를 수락하면서 설치되지 않은 툴셋을 가리키게 된 것. 설치된 건 v143(MSVC 14.44).

**해결**
`.vcxproj`의 `PlatformToolset` 4곳(Debug/Release × Win32/x64)을 전부 `v143`으로 되돌림.

> ⚠️ **함정:** 프로젝트 속성에서 고칠 때 구성을 "모든 구성", 플랫폼을 "모든 플랫폼"으로 놓지 않으면 **현재 선택된 하나만** 바뀐다. 실제로 처음에 Debug|x64만 고쳐져서 Debug 빌드는 되는데 Release 빌드는 계속 실패했음.

---

## 10. IntelliSense 자동완성이 같은 폴더 파일만 보여줌

**증상**
`#include "` 를 칠 때 자동완성 목록에 현재 폴더 파일만 뜨고, `GameFramework/`, `Actor/` 같은 프로젝트 루트 기준 경로가 안 나옴. **빌드는 정상.**

**원인**
프로젝트 설정(`AdditionalIncludeDirectories`에 `$(ProjectDir)`)은 멀쩡했으므로 컴파일러는 경로를 잘 찾고 있었음. 문제는 **IntelliSense 캐시(`.vs` 폴더, 823MB)** 가 그 설정을 반영하지 못한 것. 이 프로젝트엔 `imgui.cpp`(약 25,000줄), `json.hpp`(단일 헤더 수만 줄)처럼 IntelliSense가 버거워하는 파일이 있어 DB가 비대해지기 쉬움.

**해결**
VS 종료 → `.vs` 폴더 삭제(`.gitignore` 대상인 순수 캐시) → 재실행하여 재인덱싱.

> 📌 **교훈:** "빌드는 되는데 에디터만 이상하다"면 코드가 아니라 IDE 캐시를 의심할 것. 둘을 구분하는 게 시간을 아낀다.

---

## 11. 중괄호 없는 `if` 때문에 조건과 무관하게 매 프레임 실행됨

**증상**
"버튼을 눌렀을 때만 켜지는" 플래그를 만들었는데, 아무것도 안 눌러도 처음부터 켜져 있었음.

**원인**
```cpp
if (ImGui::Button("위")) _testOffsetFromHome = Vector2(0, -r); _testAlignActive = true;
```
중괄호 없는 `if`는 **바로 다음 문장 하나만** 조건에 걸린다. 실제 의미는:
```cpp
if (ImGui::Button("위"))
    _testOffsetFromHome = Vector2(0, -r);   // 버튼 눌렀을 때만

_testAlignActive = true;                    // ← 조건과 무관하게 항상 실행!
```
`OnSceneGUI()`는 매 프레임 호출되므로 플래그가 첫 프레임부터 `true`가 되어 가드가 무력화됨.

**해결**
한 줄에 두 문장을 붙이지 말고 중괄호로 묶는다. **가드를 추가했는데도 증상이 그대로면, 가드가 실제로 작동하는지부터 확인할 것.**

---

## 12. 테스트 지점을 절대좌표로 저장해서, 공전하는 행성과 어긋남

**증상**
카메라 정렬 기준점을 행성 표면에 놓았는데, 시간이 지나면서 카메라가 계속 회전함.

**원인**
기준점을 **그 순간의 절대 월드 좌표**로 저장했는데, 기준이 되는 행성(지구)은 계속 공전 중이었음. 지구의 공전 속도가 `60000 × 0.02 = 1200 유닛/초`인데 행성 반지름은 `1000`이라, 방금 클릭한 직후엔 각속도가 순간적으로 `1.2 rad/s`(초당 약 68°)까지 나옴 → 카메라가 그걸 실시간으로 쫓아가며 회전.

**해결**
움직이는 대상 기준의 좌표는 **절대좌표가 아니라 상대 오프셋으로 저장**하고, 매 프레임 `대상위치 + 오프셋`으로 다시 계산해야 한다.

> 📌 이 항목은 이후 설계 전환(18번)으로 코드 자체가 삭제됐지만, **"움직이는 것에 붙는 좌표는 상대값으로"** 라는 교훈은 2주차 우주선 물리에서 그대로 적용된다.

---

## 13. 카메라 보간을 만들었는데 여전히 순간이동함

**증상**
`UpdateFollow()`에 지수감쇠 보간을 넣었는데도 행성 클릭 시 카메라가 뚝 하고 순간이동.

**원인**
보간을 새로 만들면서, **기존의 즉시 대입 코드를 지우지 않았음.**
```cpp
// MainWorld::Update() — 예전 코드가 그대로 남아있었다
if (_selected != nullptr)
    _camera.SetPosition(_selected->GetCenterPos());   // _position에 즉시 대입
```
`UpdateFollow()`가 애써 보간한 `_position`을 매 프레임 이 줄이 덮어써버림.

**해결**
옛 코드 삭제. **기능을 "교체"할 때는 새 코드를 넣는 것과 옛 코드를 지우는 것이 한 쌍**이다.

---

## 14. 슬라이더를 움직여도 속도가 안 변함

**증상**
`_followK` 필드를 만들고 ImGui 슬라이더까지 연결했는데, 아무리 조절해도 카메라 추적 속도가 그대로.

**원인**
```cpp
float a = std::exp(-2.f * deltaTime);   // ← 상수가 하드코딩된 채로 남아있음
```
슬라이더는 `_followK`를 바꾸는데 계산은 `2.f`를 쓰고 있었음.

**해결**
`-_followK * deltaTime`으로 교체. **"조절 가능하게 만들었다"는 건 변수 선언·UI 연결·실제 사용처 교체 세 가지가 다 끝나야 완성이다.**

---

## 15. ImGui 슬라이더로 회전시키면 돌아갔다가 제자리로 돌아옴

**증상**
`TargetRotation`을 조절해도 드래그 중엔 화면이 반응 없다가, `Rotation`을 직접 조절하면 회전했다가 스르륵 원위치로 돌아감.

**원인 두 가지가 겹침**

1. **ImGui를 만지는 동안 보간이 멈춤** — 2번 항목에서 만든 `io.WantCaptureMouse` 가드가 `UpdateFollow()`까지 막고 있었음. 슬라이더를 드래그하는 순간이 정확히 `WantCaptureMouse == true`인 때라, 값은 바뀌는데 보간은 안 돌아감.
2. **`Rotation` 슬라이더가 매 프레임 덮어써짐** — `_rotation = LerpAngle(_rotation, _targetRotation, ...)`가 매 프레임 실행되므로, `_rotation`을 직접 바꿔도 다음 프레임에 `_targetRotation` 쪽으로 다시 끌려감.

**해결**
```cpp
void Camera::Update(float deltaTime)
{
    UpdateFollow(deltaTime);   // ← 가드 밖으로. 마우스 상태와 무관하게 항상 보간

    if (!_INPUT.IsMouseInsideWindow(...) or io.WantCaptureMouse) { ... return; }
    ...
}
```
그리고 `_rotation`은 **결과값(읽기 전용)**, `_targetRotation`만 조작하도록 UI를 정리.

> 📌 **교훈:** 입력 가드(마우스가 UI 위에 있는가)와 상태 갱신(보간)은 **성격이 다른 로직**이다. 같은 가드로 묶으면 "입력이 없을 때 상태도 멈추는" 부작용이 생긴다.

---

## 16. `targetRotation = 0`인데 `rotation`이 `6.283`에서 멈춤 — 버그가 아니었음

**증상**
목표를 `0`으로 뒀는데 `_rotation`이 `6.283 rad`에서 수렴하고 더 안 움직임.

**원인 (정상 동작)**
`6.283 rad = 2π = 360°`이므로 `0 rad`과 **화면상 완전히 같은 방향**이다. `LerpAngle`이 최단경로로 접기 때문에:
```
delta = 0 - 6.283 = -6.283
      → atan2(sin(-2π), cos(-2π)) = atan2(≈0, ≈1) ≈ 0
```
"이미 목표와 같은 방향"이라 판단해 정지한 것. **한 바퀴를 헛돌지 않는 게 최단경로 보간의 목적**이므로 의도대로 동작한 것이다.

**정리 (선택)**
값이 계속 누적되면 디버그 창 숫자를 읽기 어렵고 `sinf`/`cosf` 정밀도도 떨어지므로, 보간 후 `(-π, π]`로 접어두면 깔끔:
```cpp
_rotation = atan2f(sinf(_rotation), cosf(_rotation));   // 화면상 변화 없음
```
> ⚠️ 이때 `Util.cpp`의 `DegreeToRadian`을 쓰면 안 된다. `[0, 2π)`로 접어서 `-0.1 rad`을 `6.18 rad`로 바꿔버려 더 헷갈린다.

---

## 17. 별 배경 때문에 프레임이 60~70으로 떨어짐

**증상**
`StarField`를 넣은 뒤 프레임 저하.

**원인**
```cpp
_starField->Init(200000, 3000000.f);   // 별 20만 개
```
매 프레임 **20만 번 순회하며 각각 행렬 변환**. 게다가 범위를 `±3,000,000`으로 크게 잡아서, 최대 줌아웃에서도 화면에 들어오는 건 전체의 **0.04%뿐** — 즉 20만 개를 돌려서 실제로 그리는 건 100개 남짓이었음. Debug 빌드(`/Od`, `/RTC1`, `/JMC`)라 `Vector2` 반환·복사가 인라인 없이 실제 함수 호출로 도는 것도 겹침.

**해결**
1. **숫자 조정** — 가장 먼 행성(해왕성 `240,000`)에 맞춰 범위를 줄이고 개수도 낮춤. 범위를 좁히면 같은 개수로 화면에 더 많이 보이므로 **가벼우면서 더 잘 보인다.**
2. **월드 좌표에서 먼저 컷** — 화면 네 모서리를 `ScreenToWorld`로 역변환해 보이는 월드 AABB를 한 번만 구하고, 각 별은 **행렬 변환 전에** 비교 4번으로 걸러낸다.
3. 부족하면 격자 분할로 `O(전체)` → `O(보이는 것)`.

> 📌 **성능은 Release로 측정할 것.** Debug 빌드 수치는 실제 성능이 아니다.

---

## 18. ⭐ 설계 전환 — 표면 정렬(B-8)을 버리고 World를 분리

**상황**
로드맵 Day 5는 "행성 표면 어디에서 발사해도 지평선이 수평으로 보이게" 하는 표면 정렬 회전(`θ = -π/2 - atan2(u.y, u.x)`)을 구현하는 날이었다. 실제로 구현에 착수해서 4방향 검증 UI까지 만들었다.

**문제**
구현 도중 게임 흐름을 다시 정리해보니 실제로 만들려는 것은:
```
평면 바닥에서 발사 → 방향키 조종 → 대기권 통과 → 화면 전환 → 태양계에서 궤도 진입
```
즉 **발사 구간은 평면 바닥**이고, 태양계는 그 뒤에 나오는 별개의 장면이었다. 평면 바닥이면 지평선이 기울어질 일이 없으므로 표면 정렬이 보정할 대상 자체가 없다.

**결정**
- **발사 World**(평면 바닥·우주선·대기권)와 **우주 World**(태양계·궤도)를 **서로 다른 `World`로 분리**한다.
- 표면 정렬(B-8) 관련 코드는 전부 삭제한다.
- 다만 그 과정에서 만든 범용 기능(`LerpAngle`, 카메라 회전 보간)은 남겨서 B-9에서 재사용한다.

**회고에 쓸 판단 근거**
설계 문서가 "둥근 행성 표면에서 발사"를 전제로 쓰여 있었기 때문에 표면 정렬이 필수 항목으로 잡혀 있었다. **전제가 바뀌면 그 위에 세워진 항목도 다시 검토해야 한다**는 것을 실제로 코드를 절반쯤 만든 뒤에야 깨달았다. 문서의 Day 항목을 순서대로 따라가는 것과, 그 항목이 지금 만들려는 게임에 실제로 필요한지 묻는 것은 별개의 일이다.

> 📌 **비용:** 표면 정렬에 쓴 시간은 버려졌지만, 그 과정에서 만든 `LerpAngle`·회전 보간·`atan2` 이해는 그대로 남았다. 그리고 이 판단을 2주차에 했다면 훨씬 비쌌을 것이다.
