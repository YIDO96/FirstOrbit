#pragma once

#include "GameFramework/UIBase.h"

class UIText : public UIBase
{
	using Super = UIBase;

public:
	UIText() {}
	UIText(const std::wstring& text) : _text(text) { }

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;

	void SetName(const std::wstring& text) { _name = WStringToString(text); }
	void SetText(const std::wstring& text) { _text = text; }
	
private:
	std::wstring _text;
};

