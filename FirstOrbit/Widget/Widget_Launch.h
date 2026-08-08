#pragma once

#include "GameFramework/UI/Widget.h"

class UIText;
class UIButton;


class Widget_Launch : public Widget
{
	using Super = Widget;
public:
	virtual void Init() override;
	virtual void Update(float deltaTime) override;

	virtual void Render(HDC hdc) override;

	virtual void OnGUI();

	void SetStateText(const std::wstring& text);
	void ReSizeStateText();

	void ResetWidget();
private:
	UIText* _launchStateText = nullptr;
	UIButton* _FireButton = nullptr;
	//UIButton* _exitButton = nullptr;
};

