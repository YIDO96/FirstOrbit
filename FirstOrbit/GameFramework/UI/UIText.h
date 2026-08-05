#pragma once

#include "GameFramework/UIBase.h"

class UIText : public UIBase
{
	using Super = UIBase;

public:
	UIText() { SetTransparent(true); }
	UIText(const std::wstring& text) : _text(text) { SetTransparent(true); }
	~UIText() { if (_hFont) ::DeleteObject(_hFont); }

	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size) override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;

	virtual void SetSize(float w, float h) override;
	void SetFontSize(float size);
	void SetName(const std::wstring& text) { _name = WStringToString(text); }
	void SetText(const std::wstring& text) { _text = text; }
	void SetTextColor(COLORREF color) { _textColor = color; }


	COLORREF GetTextColor() const { return _textColor; }
private:
	std::wstring _text;

	float _fontSize = 20.f;
	HFONT _hFont = nullptr;
	COLORREF _textColor = RGB(255, 255, 255);
};

