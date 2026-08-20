#include "pch.h"
#include "Widget_Launch.h"

#include "Core/WorldManager.h"
#include "Core/ResourceManager.h"

#include "GameFramework/World.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/UI/UIText.h"
#include "GameFramework/UI/UIButton.h"
#include "GameFramework/UI/UISlider.h"
#include "GameFramework/Texture.h"

#include "GameMode/LaunchGameMode.h"

#include "Actor/ASpaceship.h"

void Widget_Launch::Init()
{
	Super::Init();

	// 이 위젯의 자식들은 전부 오른쪽 사이드 패널(GImGuiPanelWidth 기준) 좌표계에 그려진다.
	UIBase::SetActiveUISpace(true);

	// 스케치 레이아웃: 타이틀 → 상태 텍스트 → 점수/콤보(자리만) → Fire 버튼 → 연료바
	_titleText = AddChild<UIText>();
	_titleText->Init(EAnchor::Top, EPivot::Top, Vector2(0.f, 15.f), Vector2(0, 0), L"First Orbit");
	_titleText->SetFontSize(28.f);

	_launchStateText = AddChild<UIText>();
	_launchStateText->Init(EAnchor::Top, EPivot::Top, Vector2(0.f, 55.f), Vector2(0, 0), L"Awaiting Launch");
	_launchStateText->SetFontSize(20.f);

	_altitudeText = AddChild<UIText>();
	_altitudeText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(20.f, 100.f), Vector2(0, 0), L"Altitude : 0");
	_altitudeText->SetFontSize(16.f);

	_horizontalVelocityText = AddChild<UIText>();
	_horizontalVelocityText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(20.f, 125.f), Vector2(0, 0), L"HorizontalVelocity : 0");
	_horizontalVelocityText->SetFontSize(16.f);

	// TODO: 점수/콤보 시스템 자체는 아직 미확정(브레인스토밍 중) — 자리만 잡아둔 텍스트, 실제 값 연결은 나중에.
	_scoreText = AddChild<UIText>();
	_scoreText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(20.f, 160.f), Vector2(0, 0), L"점수 : 0");
	_scoreText->SetFontSize(16.f);

	_comboText = AddChild<UIText>();
	_comboText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(20.f, 185.f), Vector2(0, 0), L"콤보 : 0");
	_comboText->SetFontSize(16.f);

	// TODO(스케치의 보라색 버튼 3개): 어떤 파워업/보너스가 될지 아직 결정 안 됨 — 기능 없는 빈 버튼을
	// 미리 만들어두면 죽은 코드가 되니, 실제 기획이 정해지면 그때 여기에 추가할 것.

	// 키 조작 안내. LaunchWorld는 Enter로 자동조종 발사가 걸리기 때문에(ASpaceship::Input()이
	// _autoPilot이면 수동 입력을 무시함) 실제로 플레이어가 쓰는 키는 이 둘뿐이다.
	_keyGuideTitle = AddChild<UIText>();
	_keyGuideTitle->Init(EAnchor::LeftTop, EPivot::Left, Vector2(20.f, 230.f), Vector2(0, 0), L"조작");
	_keyGuideTitle->SetFontSize(16.f);
	_keyGuideTitle->SetTextColor(RGB(180, 180, 180));

	_keyLaunchText = AddChild<UIText>();
	_keyLaunchText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(28.f, 256.f), Vector2(0, 0), L"발사 : Enter");
	_keyLaunchText->SetFontSize(16.f);

	_keyResetText = AddChild<UIText>();
	_keyResetText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(28.f, 280.f), Vector2(0, 0), L"재시작 : R");
	_keyResetText->SetFontSize(16.f);
	_keyResetText->SetTextColor(RGB(150, 150, 150));

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");

	//_FireButton = AddChild<UIButton>();
	//_FireButton->SetTexture(buttonTex);
	//Vector2 FireButtonSize = buttonTex->GetTextureSize() * 1.5f;
	//_FireButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, 0), FireButtonSize);
	//_FireButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 0), FireButtonSize, L"Fire");
	//_FireButton->SetOnClick([this]()
	//	{
	//		LaunchGameMode* gm = _ownerWorld->GetGameMode<LaunchGameMode>();
	//		gm->ChangeLaunchState(ELaunchState::Countdown);
	//		_FireButton->SetActive(false);
	//	});


	_fuelSlider = AddChild<UISlider>();
	_fuelSlider->Init(EAnchor::LeftBottom, EPivot::LeftBottom, Vector2(20.f, -20.f), Vector2(150.f, 20.f));

	UIBase::SetActiveUISpace(false);   // 원상 복구 (다음에 만들어질 다른 UI가 영향받지 않도록)
}

void Widget_Launch::Update(float deltaTime)
{
	Super::Update(deltaTime);

	LaunchGameMode* gm = _ownerWorld->GetGameMode<LaunchGameMode>();
	if (gm->GetLaunchState() == ELaunchState::Ascent)
	{
		_altitudeText->SetText(L"Altitude : " + _altiText);
		_altitudeText->SetSize(0, 0);

		_horizontalVelocityText->SetText(L"HorizontalVelocity : " + _hvText);
		_altitudeText->SetSize(0, 0);
	}
	
}

void Widget_Launch::Render(HDC hdc)
{
	Super::Render(hdc);

	// TODO(연료 게이지):
	// - 화면 고정 위치(예: 좌하단)에 배경 바 하나, 그 위에 _fuelRatio만큼 폭이 줄어드는 바 하나
	// - LaunchWorld::Render()가 바닥선을 그릴 때 쓴 CreatePen/Rectangle 스타일 참고
	// - 연료가 30% 밑으로 내려가면 색을 바꾸는 것도 좋은 피드백 (선택)
}

void Widget_Launch::OnGUI()
{
}

void Widget_Launch::SetFuelRatio(float ratio)
{
	_fuelSlider->SetRatio(ratio);
}

void Widget_Launch::SetStateText(const std::wstring& text)
{
	_launchStateText->SetText(text);
}

void Widget_Launch::ReSizeStateText()
{
	_launchStateText->SetSize(0 ,0);
}

void Widget_Launch::ResetWidget()
{
	//_FireButton->SetActive(true); 
}

void Widget_Launch::BindShip(ASpaceship* ship)
{
	if (not ship) return;

	ship->SetOnAltitudeChanged([this](float newAltitude)
		{
			this->OnAltitudeChanged(newAltitude);
		});

	ship->SetOnHVChanged([this](float newHV)
		{
			this->OnHVChanged(newHV);
		});
}

void Widget_Launch::OnAltitudeChanged(float newAltitude)
{
	_altiText = to_wstring(static_cast<int>(newAltitude));;
}

void Widget_Launch::OnHVChanged(float newHV)
{
	wchar_t buffer[32];
	// 소수점 첫째자리까지 포맷팅
	std::swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"%.1f", newHV);

	_hvText = buffer;
}
