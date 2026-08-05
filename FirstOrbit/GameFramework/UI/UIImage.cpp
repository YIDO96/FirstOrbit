#include "pch.h"
#include "UIImage.h"

void UIImage::Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size)
{
}

void UIImage::Update(float deltaTime)
{
	Super::Update(deltaTime);
}

void UIImage::Render(HDC hdc)
{
	if (!_isActive || !_texture) return;

	Super::Render(hdc);

	// _texture->Render(hdc, _x, _y, _width, _height); // 텍스처 렌더링 호출
}
