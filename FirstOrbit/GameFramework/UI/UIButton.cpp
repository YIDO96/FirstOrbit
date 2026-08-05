#include "pch.h"
#include "UIButton.h"

#include "Core/InputManager.h"
#include "GameFramework/Texture.h"

void UIButton::Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size)
{


    SetAnchor(anchor);
    SetPivot(pivot);


    SetSize(size.x, size.y);
    SetPos(pos.x, pos.y);

    originSize = Vector2(_width, _height);
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
        SetSize(originSize.x * 1.1f, originSize.y * 1.1f);

        if (_INPUT.GetButtonDown(KeyType::LeftMouse))
        {
            if (_onClick) _onClick();
        }
    }
    else
    {
        SetSize(originSize.x, originSize.y);
    }
}

void UIButton::Render(HDC hdc)
{
    if (!_isActive) return;
    // 버튼 배경/텍스처 및 텍스트 렌더링
    if (!_texture) return;

    _texture->Render(hdc, Vector2(_finalX, _finalY), Vector2(), Vector2(), Vector2(_width, _height));

}
