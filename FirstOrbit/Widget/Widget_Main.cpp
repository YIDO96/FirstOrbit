#include "pch.h"
#include "Widget_Main.h"

#include "Core/GameInstance.h"
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

	// 이 위젯의 자식들은 전부 오른쪽 사이드 패널(GImGuiPanelWidth 기준) 좌표계에 그려진다.
	UIBase::SetActiveUISpace(true);

	// 스케치 레이아웃: 타이틀 → 상태 텍스트 → (우주선 버튼 목록, 4단계에서 채움) → 연료바/Refuel
	_titleText = AddChild<UIText>();
	_titleText->Init(EAnchor::Top, EPivot::Top, Vector2(0.f, 15.f), Vector2(0, 0), L"First Orbit");
	_titleText->SetFontSize(28.f);

	static const float kSpeedOptions[4] = { 1.f, 10.f, 20.f, 100.f };
	static const wchar_t* kSpeedLabels[4] = { L"x1", L"x10", L"x20", L"x100" };

	for (int i = 0; i < kLineCount; ++i)
	{
		_lines[i] = AddChild<UIText>();
		_lines[i]->Init(EAnchor::LeftTop, EPivot::LeftTop, Vector2(20.f, 60.f + i * 24.f), Vector2(0, 0),L"");
		_lines[i]->SetFontSize(18.f);
	}

	_fuelSlider = AddChild<UISlider>();
	_fuelSlider->Init(EAnchor::LeftBottom, EPivot::LeftBottom, Vector2(20.f, -20.f), Vector2(150.f, 20.f));

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");
	_refuelButton = AddChild<UIButton>();
	_refuelButton->SetTexture(buttonTex);
	Vector2 btnSize = buttonTex->GetTextureSize() * 0.8f;
	_refuelButton->Init(EAnchor::LeftBottom, EPivot::LeftBottom, Vector2(20.f, -55.f), btnSize);   // 연료 게이지 바로 위
	_refuelButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 5), btnSize, L"Refuel", 16.f);
	_refuelButton->SetOnClick([this]()
		{
			if (_ship) _ship->ReFuelFull();
		});

	//Vector2 smallBtnSize = btnSize * 0.7f;
	Vector2 smallBtnSize = Vector2(btnSize.x * .5f, btnSize.y);
	constexpr float kSpeedButtonSpacing = 90.f;
	for (int i = 0; i < 4; ++i)
	{
		_speedButtons[i] = AddChild<UIButton>();
		_speedButtons[i]->SetTexture(buttonTex);
		constexpr float kSpeedButtonY = 190.f + kMaxActorButtons * 32.f + 20.f;   // = 562
		float xOffset = (i) * kSpeedButtonSpacing;
		_speedButtons[i]->Init(EAnchor::Top, EPivot::Top, Vector2(-90.f + i * 60.f, kSpeedButtonY), smallBtnSize);
		_speedButtons[i]->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 3), smallBtnSize, kSpeedLabels[i], 14.f);

		float speed = kSpeedOptions[i];
		_speedButtons[i]->SetOnClick([speed]()
			{
				GAME.SetTimeScale(speed);
			});
	}

	// Refuel이 연료바 위쪽으로 옮겨졌으니, 헤딩 인디케이터(우주선 방향)가 그 자리(우측 아래)로 들어간다.
	_headingIndicator = AddChild<UIHeadingIndicator>();
	_headingIndicator->Init(EAnchor::RightBottom, EPivot::RightBottom, Vector2(-20.f, -20.f), Vector2(50.f, 50.f));

	// 우주선/태양계/블랙홀 버튼 자체는 ShowActorButtons()에서 만든다 (Enter() 시점에 액터가 다 생긴
	// 뒤 한 번만 호출됨 — 여기 Init()에선 아직 우주선/행성이 스폰되기 전이라 만들 수가 없음).

	UIBase::SetActiveUISpace(false);   // 원상 복구 (다음에 만들어질 다른 UI가 영향받지 않도록)
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

void Widget_Main::ShowActorButtons(const vector<AActor*>& actors)
{
	// Enter()에서 액터(우주선/행성/블랙홀)가 다 스폰된 뒤 단 한 번만 호출된다.
	// Init()이 아니라 여기서 버튼을 만들기 때문에, 패널 좌표계 토글을 여기서도 다시 해줘야 한다.
	UIBase::SetActiveUISpace(true);

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");
	Vector2 btnSize = buttonTex->GetTextureSize() * 0.8f;

	_actorButtonCount = min((int)actors.size(), kMaxActorButtons);

	for (int i = 0; i < _actorButtonCount; ++i)
	{
		_actorButtons[i] = AddChild<UIButton>();
		_actorButtons[i]->SetTexture(buttonTex);
		_actorButtons[i]->Init(EAnchor::LeftTop, EPivot::LeftTop, Vector2(20.f, 190.f + i * 32.f), btnSize);

		wstring text = StringToWString(actors[i]->GetName());
		_actorButtons[i]->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 5), btnSize, text, 14.f);

		_actorButtonTargets[i] = actors[i];
		_actorButtons[i]->SetOnClick([this, i]()
			{
				if (_onActorSelected) _onActorSelected(_actorButtonTargets[i]);
			});
	}

	UIBase::SetActiveUISpace(false);
}

void Widget_Main::ShowMoonButtons(const vector<pair<AActor*, AActor*>>& moonParentPairs)
{
	UIBase::SetActiveUISpace(true);

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");
	Vector2 btnSize = buttonTex->GetTextureSize() * 0.8f;
	Vector2 smallSize = btnSize;   // 지구 버튼과 비슷한 크기로

	for (const auto& pair : moonParentPairs)
	{
		if (_moonButtonCount >= kMaxMoonButtons) break;

		AActor* moon = pair.first;
		AActor* parent = pair.second;

		// 부모 행성이 몇 번째 행(버튼)에 있는지 찾아서, 그 행의 오른쪽에 작은 버튼을 붙인다.
		int parentIndex = -1;
		for (int i = 0; i < _actorButtonCount; ++i)
		{
			if (_actorButtonTargets[i] == parent)
			{
				parentIndex = i;
				break;
			}
		}
		if (parentIndex < 0) continue;   // 부모 버튼이 없으면(목록에서 빠졌으면) 스킵

		int slot = _moonButtonCount++;
		_moonButtons[slot] = AddChild<UIButton>();
		_moonButtons[slot]->SetTexture(buttonTex);
		_moonButtons[slot]->Init(EAnchor::LeftTop, EPivot::LeftTop,
			Vector2(20.f + btnSize.x + 8.f, 190.f + parentIndex * 32.f), smallSize);

		wstring text = StringToWString(moon->GetName());
		_moonButtons[slot]->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 5), smallSize, text, 14.f);

		_moonButtonTargets[slot] = moon;
		_moonButtons[slot]->SetOnClick([this, slot]()
			{
				if (_onActorSelected) _onActorSelected(_moonButtonTargets[slot]);
			});
	}

	UIBase::SetActiveUISpace(false);
}

void Widget_Main::UpdateActorButtonStates(bool shipPicked, const vector<APlanet*>& reachable)
{
	for (int i = 0; i < _actorButtonCount; ++i)
	{
		AActor* actor = _actorButtonTargets[i];

		// 우주선이 안 픽힌 상태면 전부 활성(그냥 포커스용 버튼) — 우주선 버튼 자신도 항상 활성.
		if (not shipPicked or actor == _ship)
		{
			_actorButtons[i]->SetInteractable(true);
			continue;
		}

		// 우주선이 픽힌 상태: 행성이면 "지금 갈 수 있는지"로 활성/비활성 판정.
		// 행성이 아닌 것(블랙홀 등, 전이 대상이 아닌 액터)은 그냥 포커스용으로 항상 활성.
		APlanet* planet = dynamic_cast<APlanet*>(actor);
		bool interactable = planet ? (find(reachable.begin(), reachable.end(), planet) != reachable.end()) : true;
		_actorButtons[i]->SetInteractable(interactable);
	}
}
