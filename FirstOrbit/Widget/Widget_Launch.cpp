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

	_launchStateText = AddChild<UIText>();
	_launchStateText->Init(EAnchor::Top, EPivot::Center, Vector2(0, 150), Vector2(0, 0), L"Awaiting Launch");
	_launchStateText->SetFontSize(40.f);

	_altitudeText = AddChild<UIText>();
	_altitudeText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(0, 10), Vector2(0, 0), L"Altitude : 0");
	_altitudeText->SetFontSize(30.f);

	_horizontalVelocityText = AddChild<UIText>();
	_horizontalVelocityText->Init(EAnchor::LeftTop, EPivot::Left, Vector2(0, 40), Vector2(0, 0), L"HorizontalVelocity : 0");
	_horizontalVelocityText->SetFontSize(30.f);

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");

	_FireButton = AddChild<UIButton>();
	_FireButton->SetTexture(buttonTex);
	Vector2 FireButtonSize = buttonTex->GetTextureSize() * 1.5f;
	_FireButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, 0), FireButtonSize);
	_FireButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 0), FireButtonSize, L"Fire");
	_FireButton->SetOnClick([this]()
		{
			LaunchGameMode* gm = _ownerWorld->GetGameMode<LaunchGameMode>();
			gm->ChangeLaunchState(ELaunchState::Countdown);
			_FireButton->SetActive(false);
		});


	_fuelSlider = AddChild<UISlider>();
	_fuelSlider->Init(EAnchor::LeftBottom, EPivot::LeftBottom, Vector2(20.f, -20.f), Vector2(150.f, 20.f));
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
	_FireButton->SetActive(true);
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
