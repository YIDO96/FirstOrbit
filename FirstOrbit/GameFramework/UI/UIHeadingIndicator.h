#pragma once

#include "GameFramework/UIBase.h"

class UIHeadingIndicator : public UIBase
{
	using Super = UIBase;
public:
	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text = L"") override;
	virtual void Render(HDC hdc) override;

	void SetHeading(float radian) { _heading = radian; }   // 0 = 화면 위쪽

private:
	float _heading = 0.f;
};

