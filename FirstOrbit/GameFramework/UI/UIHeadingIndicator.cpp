#include "pch.h"
#include "UIHeadingIndicator.h"

void UIHeadingIndicator::Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text)
{
	SetAnchor(anchor);
	SetPivot(pivot);
	SetSize(size.x, size.y);
	SetPos(pos.x, pos.y);
}

void UIHeadingIndicator::Render(HDC hdc)
{
	if (!_isActive) return;

	Vector2 center(_finalX + _width / 2.f, _finalY + _height / 2.f);
	float r = min(_width, _height) / 2.f;

	// 로컬 기준(0도 = 위쪽) 화살표 세 꼭짓점
	Vector2 localPoints[3] = {
		Vector2(0.f, -r),
		Vector2(-r * 0.6f, r * 0.6f),
		Vector2(r * 0.6f, r * 0.6f)
	};

	POINT pts[3];
	for (int i = 0; i < 3; ++i)
	{
		Vector2 rotated = localPoints[i].Rotate(_heading);
		pts[i].x = (long)(center.x + rotated.x);
		pts[i].y = (long)(center.y + rotated.y);
	}

	// 배경 원
	HBRUSH bgBrush = ::CreateSolidBrush(RGB(30, 30, 30));
	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, bgBrush);
	HPEN circlePen = ::CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
	HPEN oldPen = (HPEN)::SelectObject(hdc, circlePen);
	::Ellipse(hdc, (int)(center.x - r), (int)(center.y - r), (int)(center.x + r), (int)(center.y + r));

	// 방향 화살표
	HBRUSH arrowBrush = ::CreateSolidBrush(RGB(255, 255, 0));
	::SelectObject(hdc, arrowBrush);
	::Polygon(hdc, pts, 3);

	::SelectObject(hdc, oldBrush);
	::SelectObject(hdc, oldPen);
	::DeleteObject(bgBrush);
	::DeleteObject(circlePen);
	::DeleteObject(arrowBrush);
}
