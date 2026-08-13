#pragma once

#include "GameFramework/UIBase.h"

class UISlider : public UIBase
{
	using Super = UIBase;

public:

	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text = L"") override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

	void SetRatio(float ratio) { _ratio = clamp(ratio, 0.f, 1.f); }
	void SetFillColor(COLORREF color) { _fillColor = color; }
	void SetBackColor(COLORREF color) { _backColor = color; }


private:
	float _ratio = 1.f;
	COLORREF _fillColor = RGB(80, 220, 100);
	COLORREF _backColor = RGB(40, 40, 40);
};

