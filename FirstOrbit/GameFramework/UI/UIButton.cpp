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

    // 비활성(흐림) 상태면 호버 확대/클릭 반응 없이 그대로 원래 크기로 고정.
    if (!_isInteractable)
    {
        SetSize(_originSize.x, _originSize.y);
        if (_text)
        {
            _text->SetFontSize(_originFontSize);
            _text->InitButton(_textAnchor, _textPivot, Vector2(_textPos.x, _textPos.y - 5), _textSize);
            _text->Update(deltaTime);
        }
        return;
    }

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

    if (!_isInteractable)
    {
        // 비활성 상태: 버튼 위에 반투명 검정을 덮어서 흐리게 보이게 한다.
        // (SourceConstantAlpha만 쓰고 AlphaFormat은 0으로 둬서, 원본 픽셀의 알파는 무시하고
        //  1x1 불투명 검정을 균일한 비율로 얹는 방식 — Glow.cpp의 AC_SRC_ALPHA 방식과 다름)
        static HDC dimHdc = nullptr;
        static HBITMAP dimBmp = nullptr;
        if (!dimHdc)
        {
            dimHdc = CreateCompatibleDC(hdc);
            dimBmp = CreateCompatibleBitmap(hdc, 1, 1);
            SelectObject(dimHdc, dimBmp);
            SetPixel(dimHdc, 0, 0, RGB(0, 0, 0));
        }

        BLENDFUNCTION bf = { 0 };
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = 150;   // 0~255, 클수록 더 어둡게 덮임
        bf.AlphaFormat = 0;

        AlphaBlend(hdc, (int)_finalX, (int)_finalY, (int)_width, (int)_height, dimHdc, 0, 0, 1, 1, bf);
    }
}
