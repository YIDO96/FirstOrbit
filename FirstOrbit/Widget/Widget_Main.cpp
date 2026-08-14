#include "pch.h"
#include "Widget_Main.h"

#include "Core/ResourceManager.h"

#include "Gameframework/Texture.h"

#include "GameFramework/UI/UIText.h"
#include "GameFramework/UI/UISlider.h"
#include "GameFramework/UI/UIButton.h"
#include "GameFramework/UI/UIHeadingIndicator.h"

#include "Actor/ASpaceship.h"
#include "Actor/APlanet.h"


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

	for (int i = 0; i < kMaxPlanetButtons; ++i)
	{
		_planetButtons[i] = AddChild<UIButton>();
		_planetButtons[i]->SetTexture(buttonTex);
		_planetButtons[i]->Init(EAnchor::RightTop, EPivot::Center, Vector2(-100.f, 30.f * (i + 1)), btnSize);
		_planetButtons[i]->SetActive(false);
	}
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

void Widget_Main::ShowPlanetButtions(const vector<class APlanet*>& planets)
{
	// TODO:
	// 1. planets를 돌면서 _planetButtons[i]에 SetActive(true) + SetText(..., StringToWString(planets[i]->GetName()), ...)
	// 2. _planetButtonTargets[i] = planets[i] 로 매핑 기억
	// 3. SetOnClick([this, i]() { if (_onPlanetSelected) _onPlanetSelected(_planetButtonTargets[i]); })
	//    (i를 값으로 캡처하는 이유는 위 인사이트 3번 참고)
	// 4. planets.size()보다 인덱스가 큰 나머지 버튼들은 SetActive(false)로 꺼두기 (재사용 대비)

	int count = min((int)planets.size(), kMaxPlanetButtons);

	for (int i = 0; i < count; ++i)
	{
		wstring text = StringToWString(planets[i]->GetName());

		_planetButtons[i]->SetActive(true);
		_planetButtons[i]->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 5), _planetButtons[i]->GetSize(), text, 16.f);

		_planetButtonTargets[i] = planets[i];

		_planetButtons[i]->SetOnClick([this, i]()
			{
				if (_onPlanetSelected) _onPlanetSelected(_planetButtonTargets[i]);
			});
	}

	for (int i = count; i < kMaxPlanetButtons; ++i)
	{
		// TODO: 나머지는 SetActive(false)
		_planetButtons[i]->SetActive(false);
	}
}

void Widget_Main::HidePlanetButtons()
{
	for (UIButton* planetButton : _planetButtons)
	{
		planetButton->SetActive(false);
	}
}
