#include "pch.h"
#include "Widget_Launch.h"

#include "Core/WorldManager.h"
#include "Core/ResourceManager.h"

#include "GameFramework/World.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/UI/UIText.h"
#include "GameFramework/UI/UIButton.h"
#include "GameFramework/Texture.h"

#include "GameMode/LaunchGameMode.h"

void Widget_Launch::Init()
{
	Super::Init();

	_launchStateText = AddChild<UIText>();
	_launchStateText->SetText(L"Awaiting Launch");
	_launchStateText->Init(EAnchor::Top, EPivot::Center, Vector2(0, 100), Vector2(0, 0));
	_launchStateText->SetFontSize(40.f);


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
}

void Widget_Launch::Update(float deltaTime)
{
	Super::Update(deltaTime);
}

void Widget_Launch::Render(HDC hdc)
{
	Super::Render(hdc);
}

void Widget_Launch::OnGUI()
{
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
