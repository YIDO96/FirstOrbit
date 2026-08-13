#include "pch.h"
#include "Widget_Main.h"

#include "Core/ResourceManager.h"

#include "Gameframework/Texture.h"

#include "GameFramework/UI/UIText.h"
#include "GameFramework/UI/UISlider.h"
#include "GameFramework/UI/UIButton.h"
#include "GameFramework/UI/UIHeadingIndicator.h"

#include "Actor/ASpaceship.h"

void Widget_Main::Init()
{
	Super::Init();

	for (int i = 0; i < kLineCount; ++i)
	{
		_lines[i] = AddChild<UIText>();
		_lines[i]->Init(EAnchor::LeftTop, EPivot::LeftTop, Vector2(20.f, 20.f + i * 26.f), Vector2(0, 0),L"");
		_lines[i]->SetFontSize(20.f);
	}

	_fuelSlider = AddChild<UISlider>();
	_fuelSlider->Init(EAnchor::LeftBottom, EPivot::LeftBottom, Vector2(20.f, -20.f), Vector2(150.f, 20.f));

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");
	_refuelButton = AddChild<UIButton>();
	_refuelButton->SetTexture(buttonTex);
	Vector2 btnSize = buttonTex->GetTextureSize() * 0.8f;
	_refuelButton->Init(EAnchor::LeftBottom, EPivot::LeftBottom, Vector2(200.f, -20.f), btnSize);   // 연료 게이지 오른쪽 옆
	_refuelButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 5), btnSize, L"Refuel", 16.f);
	_refuelButton->SetOnClick([this]()
		{
			if (_ship) _ship->ReFuelFull();
		});

	_headingIndicator = AddChild<UIHeadingIndicator>();
	_headingIndicator->Init(EAnchor::RightBottom, EPivot::RightBottom, Vector2(-20.f, -20.f), Vector2(60.f, 60.f));
}

void Widget_Main::SetLine(int index, const std::wstring& text)
{
	if (index < 0 or index >= kLineCount) return;
	_lines[index]->SetText(text);
}

void Widget_Main::SetFuelRatio(float ratio)
{
	_fuelSlider->SetRatio(ratio);
}

void Widget_Main::SetHeading(float radian)
{
	_headingIndicator->SetHeading(radian);
}
