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

	void ShowPlanetButtions(const vector<class APlanet*>& planets);
	void HidePlanetButtons();
	void SetOnPlanetSelected(function<void(class APlanet*)> callback) { _onPlanetSelected = callback; }

public:
	static constexpr int kLineCount = 5;
private:
	class ASpaceship* _ship = nullptr;

	UIText* _lines[kLineCount] = {};
	UISlider* _fuelSlider = nullptr;
	UIButton* _refuelButton = nullptr;
	UIHeadingIndicator* _headingIndicator = nullptr;

	static constexpr int kMaxPlanetButtons = 9;   // 태양 포함 8행성 + 여유 1
	class UIButton* _planetButtons[kMaxPlanetButtons] = {};
	class APlanet* _planetButtonTargets[kMaxPlanetButtons] = {};   // 버튼 인덱스 → 행성 매핑
	std::function<void(class APlanet*)> _onPlanetSelected = nullptr;
};

