#include "pch.h"
#include "UCircleColliderComponent.h"

#include "GameFramework/AActor.h"

void UCircleColliderComponent::Init()
{
	UColliderComponent::Init();

	// 반지름이 명시되지 않았으면 액터 크기(가로)의 절반을 기본값으로 삼는다.
	if (_radius <= 0.f)
		_radius = _owner->GetSize().x / 2.f;
}

void UCircleColliderComponent::Update()
{
	// 원은 회전에 영향을 받지 않으므로 중심 ± 반지름으로 바로 바운딩 박스를 구한다.
	Vector2 center = _owner->GetCenterPos();
	_aabb.min = Vector2(center.x - _radius, center.y - _radius);
	_aabb.max = Vector2(center.x + _radius, center.y + _radius);
}
