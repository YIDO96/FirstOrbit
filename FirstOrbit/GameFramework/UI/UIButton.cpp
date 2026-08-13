#include "pch.h"
#include "UIButton.h"

#include "Core/InputManager.h"
#include "GameFramework/Texture.h"
#include "GameFramework/UI/UIText.h"

void UIButton::Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text)
{


    SetAnchor(anchor);
    SetPivot(pivot);


    SetSize(size.x, size.y);
    SetPos(pos.x, pos.y);

    _originSize = Vector2(_width, _height);
    _originFontSize = 40.f;
}

void UIButton::SetText(EParentAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text, float fontSize)
{
    _textAnchor = anchor;
    _textPivot = pivot;
    _textPos = pos;
    _textSize = size;

    _text = new UIText();
    _text->SetParentUI(this);
    _text->SetText(text);
    _text->InitButton(anchor, pivot, Vector2(pos.x, pos.y - 5), size);

    if (fontSize > 0.01f) _originFontSize = fontSize;
    _text->SetFontSize(_originFontSize);

    COLORREF color = RGB(42, 176, 183);
    _text->SetTextColor(color);
}

void UIButton::Update(float deltaTime)
{
    if (!_isActive) return;

    // 마우스 좌표 확인 (WinAPI GetCursorPos + ScreenToClient 등 활용)
    //POINT mousePos;
    //::GetCursorPos(&mousePos);
    // HWND 가 필요한 경우 Window handle로 ScreenToClient 변환 필요

    //bool isHovered = (mousePos.x >= _x && mousePos.x <= _x + _width &&
    //    mousePos.y >= _y && mousePos.y <= _y + _height);

    // 클릭 감지 (좌클릭)
    if (IsHoverInUI(_INPUT.GetMousePos()))
    {
        SetSize(_originSize.x * _hoverRate, _originSize.y * _hoverRate);
        
        if (_text)
        {
            _text->SetFontSize(_originFontSize * _hoverRate);
            _text->InitButton(_textAnchor, _textPivot, Vector2(_textPos.x, _textPos.y - 5), _textSize);   // 커진 버튼 기준으로 위치 재계산
        }

        if (_INPUT.GetButtonDown(KeyType::LeftMouse))
        {
            if (_onClick) _onClick();
        }
    }
    else
    {
        SetSize(_originSize.x, _originSize.y);
        if (_text)
        {
            _text->SetFontSize(_originFontSize);
            _text->InitButton(_textAnchor, _textPivot, Vector2(_textPos.x, _textPos.y - 5), _textSize);
        }
    }

    if (_text)
    {
        _text->Update(deltaTime);
    }
   
}

void UIButton::Render(HDC hdc)
{
    if (!_isActive) return;
    // 버튼 배경/텍스처 및 텍스트 렌더링
    if (!_texture) return;

    _texture->Render(hdc, Vector2(_finalX, _finalY), Vector2(), Vector2(), Vector2(_width, _height));
    if (_text)
    {
        _text->Render(hdc);
    }
}
