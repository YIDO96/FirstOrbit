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


public:
	static constexpr int kLineCount = 5;
private:
	class ASpaceship* _ship = nullptr;

	UIText* _lines[kLineCount] = {};
	UISlider* _fuelSlider = nullptr;
	UIButton* _refuelButton = nullptr;
	UIHeadingIndicator* _headingIndicator = nullptr;
};

