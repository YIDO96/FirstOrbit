#pragma once

#include "GameFramework/UI/Widget.h"

class UIText;
class UISlider;
class UIButton;
class UIHeadingIndicator;

class Widget_Main : public Widget
{
	using Super = Widget;

public:
	virtual void Init() override;

	void SetLine(int index, const std::wstring& text);
	void SetFuelRatio(float ratio);
	void SetHeading(float radian);


	void BindShip(class ASpaceship* ship) { _ship = ship; }

	// 우주선 + 태양계 행성(태양 포함) + 블랙홀을 합쳐서 버튼 목록을 한 번만 만든다 (Enter()에서 1회 호출).
	void ShowActorButtons(const vector<class AActor*>& actors);
	// 우주선이 "픽"된 상태인지에 따라 버튼들의 활성/비활성(흐림)을 갱신한다.
	// shipPicked==false면 reachable은 무시하고 전부 활성화.
	void UpdateActorButtonStates(bool shipPicked, const vector<class APlanet*>& reachable);
	void SetOnActorSelected(function<void(class AActor*)> callback) { _onActorSelected = callback; }

public:
	static constexpr int kLineCount = 5;
private:
	class ASpaceship* _ship = nullptr;

	UIText* _titleText = nullptr;
	UIText* _lines[kLineCount] = {};
	UISlider* _fuelSlider = nullptr;
	UIButton* _refuelButton = nullptr;
	UIHeadingIndicator* _headingIndicator = nullptr;

	class UIButton* _speedButtons[4] = {};   // x1(해제) 포함 4개

	static constexpr int kMaxActorButtons = 11;   // 우주선 1 + 태양계(태양+행성8) 9 + 블랙홀 1
	class UIButton* _actorButtons[kMaxActorButtons] = {};
	class AActor* _actorButtonTargets[kMaxActorButtons] = {};   // 버튼 인덱스 → 액터 매핑
	int _actorButtonCount = 0;
	std::function<void(class AActor*)> _onActorSelected = nullptr;
};

