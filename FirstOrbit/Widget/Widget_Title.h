#pragma once

#include "GameFramework/UI/Widget.h"

class UIText;

class Widget_Title : public Widget
{
	using Super = Widget;
public:
	virtual void Init() override;
	virtual void Update(float deltaTime) override;

	virtual void Render(HDC hdc) override;

	virtual void OnGUI();

private:
	UIText* _titleName = nullptr;
};

