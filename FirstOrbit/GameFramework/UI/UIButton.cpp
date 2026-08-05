#include "pch.h"
#include "UIButton.h"

void UIButton::Init()
{
}

void UIButton::Update(float deltaTime)
{
    if (!_isActive) return;

    // 마우스 좌표 확인 (WinAPI GetCursorPos + ScreenToClient 등 활용)
    POINT mousePos;
    ::GetCursorPos(&mousePos);
    // HWND 가 필요한 경우 Window handle로 ScreenToClient 변환 필요

    bool isHovered = (mousePos.x >= _x && mousePos.x <= _x + _width &&
        mousePos.y >= _y && mousePos.y <= _y + _height);

    // 클릭 감지 (좌클릭)
    if (isHovered && (::GetAsyncKeyState(VK_LBUTTON) & 0x8000))
    {
        if (_onClick) _onClick(); // 콜백 실행
    }
}

void UIButton::Render(HDC hdc)
{
    if (!_isActive) return;
    // 버튼 배경/텍스처 및 텍스트 렌더링
}
