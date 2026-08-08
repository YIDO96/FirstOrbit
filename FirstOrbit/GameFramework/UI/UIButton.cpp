#include "pch.h"
#include "UIButton.h"

#include "Core/InputManager.h"
#include "GameFramework/Texture.h"
#include "GameFramework/UI/UIText.h"

void UIButton::Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size)
{


    SetAnchor(anchor);
    SetPivot(pivot);


    SetSize(size.x, size.y);
    SetPos(pos.x, pos.y);

    _originSize = Vector2(_width, _height);
    _originFontSize = 40.f;
}

void UIButton::SetText(EParentAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text)
{
    _text = new UIText();
    _text->SetParentUI(this);
    _text->SetText(text);
    _text->InitButton(anchor, pivot, Vector2(pos.x, pos.y - 5), size);
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
