#include "pch.h"
#include "UIText.h"

void UIText::Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text)
{
	SetText(text);
	SetAnchor(anchor);
	SetPivot(pivot);


	SetSize(size.x, size.y);
	SetPos(pos.x, pos.y);
}

void UIText::InitButton(EParentAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size)
{
	_parentAnchor = anchor;

	SetParentAnchor(anchor, _parent);
	SetPivot(pivot);
	SetSize(size.x, size.y);
	SetPos(pos.x, pos.y);
}

void UIText::Update(float deltaTime)
{
}

void UIText::Render(HDC hdc)
{
	if (!_isActive) return;

	Super::Render(hdc);

	if (IsTransparent())
	{
		::SetBkMode(hdc, TRANSPARENT);
	}
	else
	{
		::SetBkMode(hdc, OPAQUE);
		::SetBkColor(hdc, GetColor());
	}

	::SetTextColor(hdc, _textColor);   // ← 글자 색

	HFONT hOldFont = nullptr;
	if (_hFont)
	{
		// 내가 만든 폰트로 교체
		hOldFont = (HFONT)::SelectObject(hdc, _hFont);
	}

	::TextOutW(hdc, static_cast<int>(_finalX), static_cast<int>(_finalY), _text.c_str(), static_cast<int>(_text.length()));


	if (hOldFont)
	{
		// 원래 HDC 폰트로 복구
		::SelectObject(hdc, hOldFont);
	}
}

void UIText::OnGUI()
{
	static char textBuffer[120] = "";
	ImGui::Text("Text : ");
	ImGui::SameLine();
	if (ImGui::InputText("##", textBuffer, IM_ARRAYSIZE(textBuffer)))
	{
		_text = CharToWStringStandard(textBuffer);
	}
}

void UIText::SetSize(float w, float h)
{
	Super::SetSize(w, h);

	if (_text.empty())
	{
		_width = 0;
		_height = 0;
		RecalculateFinalPos();
		return;
	}

	// 1. 메인 화면 기반의 임시 HDC를 빌려옵니다.
	HDC hdc = ::GetDC(NULL);

	HFONT hOldFont = nullptr;
	if (_hFont)
		hOldFont = (HFONT)::SelectObject(hdc, _hFont);

	SIZE size;
	::GetTextExtentPoint32W(hdc, _text.c_str(), static_cast<int>(_text.length()), &size);
	_width = static_cast<float>(size.cx);
	_height = static_cast<float>(size.cy);

	if (hOldFont)
		::SelectObject(hdc, hOldFont);

	RecalculateFinalPos();

	// 2. 빌려온 HDC는 반드시 해제(Release)해주어야 메모리 누수가 안 납니다!
	::ReleaseDC(NULL, hdc);
}

void UIText::SetFontSize(float size)
{
	_fontSize = size;

	// 기존 폰트가 있다면 메모리 해제
	if (_hFont) ::DeleteObject(_hFont);

	// 새 크기로 폰트 생성
	_hFont = ::CreateFontW(
		_fontSize, 0, 0, 0, FW_BOLD,
		FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		0, 0,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"맑은 고딕"
	);


	SetSize(_width, _height);
}
