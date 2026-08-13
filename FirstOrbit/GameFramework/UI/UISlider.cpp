#include "pch.h"
#include "UISlider.h"

void UISlider::Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text)
{
	SetAnchor(anchor);
	SetPivot(pivot);
	SetSize(size.x, size.y);
	SetPos(pos.x, pos.y);
}

void UISlider::Update(float deltaTime)
{
	Super::Update(deltaTime);
}

void UISlider::Render(HDC hdc)
{
	if (!_isActive) return;

	Super::Render(hdc);

	RECT back{ (LONG)_finalX, (LONG)_finalY, (LONG)(_finalX + _width), (LONG)(_finalY + _height) };

	HBRUSH backBrush = ::CreateSolidBrush(_backColor);
	::FillRect(hdc, &back, backBrush);
	::DeleteObject(backBrush);

	COLORREF fillColor = (_ratio < 0.3f) ? RGB(220, 70, 60) : _fillColor;

	RECT fill = back;
	fill.right = back.left + (LONG)(_width * _ratio);

	HBRUSH fillBrush = ::CreateSolidBrush(fillColor);
	::FillRect(hdc, &fill, fillBrush);
	::DeleteObject(fillBrush);

	HPEN pen = ::CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	HPEN oldPen = (HPEN)::SelectObject(hdc, pen);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));

	::Rectangle(hdc, back.left, back.top, back.right, back.bottom);

	::SelectObject(hdc, oldBrush);
	::SelectObject(hdc, oldPen);
	::DeleteObject(pen);
}