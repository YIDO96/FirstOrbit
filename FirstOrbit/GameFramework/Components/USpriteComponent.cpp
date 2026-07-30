#include "pch.h"
#include "USpriteComponent.h"

#include "GameFramework/AActor.h"
#include "GameFramework/Texture.h"

void USpriteComponent::Render(HDC hdc)
{
	if (!_texture) return;

	float radian = DegreeToRadian(_owner->GetDegree());
	_texture->RenderRotated(hdc, _owner->GetCenterPos(), radian);
}
