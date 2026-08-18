# UI 패널 리팩터링 4편 — TitleWorld 클릭 버그 + MainWorld 재배치

## 버그: TitleWorld 버튼을 눌러도 게임 화면 쪽에서 클릭 판정이 됨

### 원인
1~3편에서 `GameInstance::Render()`가 `UIManager::Render()`를 **패널 버퍼(`_hdcPanel`) 한
곳으로만** 그리도록 바꿨다. 그런데 `Widget_Title::Init()`는 `Widget_Main`/`Widget_Launch`와
달리 `UIBase::SetActiveUISpace(true)`를 안 켜줬었다 — 즉 Start/Exit 버튼은 여전히
"게임 뷰포트(600 기준) 좌표계"로 앵커/클릭판정 계산을 하는데, 실제로 그려지는 곳은
패널 버퍼(300 기준)였던 것.

그 결과:
- **그려지는 위치**: `EAnchor::Center` → x=300(게임 뷰포트 절반) 기준으로 계산된 좌표가
  300 폭짜리 패널 버퍼에 그대로 찍히면서 패널 오른쪽 끝에 몰리듯 표시됨
- **클릭 판정 위치**: `IsHoverInUI()`가 여전히 `GAME.GetRectRatio()`(게임 뷰포트 기준)로
  마우스 좌표를 보정 → 실제로는 **게임 화면(왼쪽 검은 영역)을 클릭해야** 버튼이 눌리는
  상태였음. 사용자가 말한 "버튼 클릭이 게임화면에서 이벤트가 들어간다"가 정확히 이 증상.

### 해결
`Widget_Title::Init()`도 `Widget_Main`/`Widget_Launch`와 똑같이 앞뒤로
`UIBase::SetActiveUISpace(true/false)`를 감싸서 패널 좌표계를 쓰게 함. 타이틀 텍스트/버튼
크기도 300px 폭에 맞게 축소(폰트 60→36, 버튼 1.5배→1.3배) 하고 세로로 스택.

**참고**: 지금 이 프로젝트는 커스텀 UI를 만드는 위젯이 `Widget_Main`/`Widget_Launch`/
`Widget_Title` 세 개뿐이라(grep으로 확인) 이 세 곳만 고치면 전부 커버됨.

## MainWorld 재배치: Refuel ↔ 헤딩 인디케이터 자리 교체
- Refuel 버튼: 연료바 오른쪽 옆 → **연료바 바로 위**로 이동 (`Vector2(20, -55)`, 연료바와 같은
  x축 정렬)
- 헤딩 인디케이터(우주선 방향): 우상단 → **우측 아래**(Refuel이 있던 자리, `RightBottom`)로 이동.
  Refuel이 비켜났으니 연료바(왼쪽 아래, 폭 150)와 안 겹침.

## 수정 파일
- `Widget/Widget_Title.cpp`
- `Widget/Widget_Main.cpp`

## 검증
Release 실행 후 확인:
- 타이틀 화면에서 실제로 보이는 Start 버튼 위치를 클릭 → LaunchWorld로 정상 전환 (클릭 판정 위치
  = 시각적 위치 일치 확인)
- MainWorld: Refuel이 연료바 위, 헤딩 인디케이터가 우측 아래에 정상 배치됨
