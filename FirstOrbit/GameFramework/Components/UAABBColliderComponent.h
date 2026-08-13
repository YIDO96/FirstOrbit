#pragma once

#include "UColliderComponent.h"

// 액터의 회전(_degree)을 반영한 사각형(박스) 콜라이더.
class UAABBColliderComponent : public UColliderComponent
{
public:
	UAABBColliderComponent(AActor* owner) : UColliderComponent(owner, ColliderType::AABB) {}

	virtual void Update() override;

	void SetSide(Vector2 size) { _size = size; }

private:
	AABB UpdateAABB2D_Standard();
	AABB UpdateAABB_WithRotation();
	Vector2 TransformVector2D(const Vector2& localPos, float angleRad, const Vector2& worldPos);
};
