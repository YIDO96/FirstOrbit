#pragma once

#include "UColliderComponent.h"

// 원형 콜라이더. 반지름을 명시하지 않으면 액터 크기(가로)의 절반을 기본값으로 쓴다.
class UCircleColliderComponent : public UColliderComponent
{
public:
	UCircleColliderComponent(AActor* owner, float radius = 0.f)
		: UColliderComponent(owner, ColliderType::Circle)
	{
		_radius = radius;
	}

	virtual void Init() override;
	virtual void UpdateAABB() override;

	void SetRadius(float radius) { _radius = radius; }
};
