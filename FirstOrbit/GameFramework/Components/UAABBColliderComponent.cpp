#include "pch.h"
#include "UAABBColliderComponent.h"

#include "GameFramework/AActor.h"

void UAABBColliderComponent::Update()
{
	_aabb = UpdateAABB_WithRotation();
}

AABB UAABBColliderComponent::UpdateAABB2D_Standard()
{
	if (!_owner) return AABB();

	AABB box;

	Vector2 pos = _owner->GetPos();
	Vector2 size = _owner->GetSize();

	box.min = pos;
	box.max = pos + size;

	return box;
}

AABB UAABBColliderComponent::UpdateAABB_WithRotation()
{
	if (!_owner) return AABB();

	AABB worldAABB;

	// 1. 월드 좌표를 절대 더하지 않은 '순수 로컬 꼭짓점'을 구한다.
	// 오브젝트의 중심을 (0,0)으로 잡기 위해 크기의 절반(half)을 이용한다.
	float halfX = _size.x / 2.0f;
	float halfY = _size.y / 2.0f;

	Vector2 corners[4];
	corners[0] = { -halfX, -halfY }; // 로컬 좌상
	corners[1] = { halfX, -halfY }; // 로컬 우상
	corners[2] = { -halfX,  halfY }; // 로컬 좌하
	corners[3] = { halfX,  halfY }; // 로컬 우하

	// 2. 각도 계산
	// 액터가 이미 들고 있는 _degree를 그대로 라디안 변환해서 쓴다.
	float angleRad = DegreeToRadian(_owner->GetDegree());

	// 3. 회전의 중심이자 기준이 될 월드 위치를 구한다.
	Vector2 ownerCenterWorldPos = _owner->GetPos() + (_owner->GetSize() / 2.0f);

	// 4. 첫 번째 꼭짓점 변환
	Vector2 transformed = TransformVector2D(corners[0], angleRad, ownerCenterWorldPos);
	worldAABB.min = transformed;
	worldAABB.max = transformed;

	// 5. 나머지 꼭짓점 순회하며 AABB 크기 결정
	for (int i = 1; i < 4; ++i)
	{
		transformed = TransformVector2D(corners[i], angleRad, ownerCenterWorldPos);

		worldAABB.min.x = min(worldAABB.min.x, transformed.x);
		worldAABB.min.y = min(worldAABB.min.y, transformed.y);
		worldAABB.max.x = max(worldAABB.max.x, transformed.x);
		worldAABB.max.y = max(worldAABB.max.y, transformed.y);
	}

	return worldAABB;
}

Vector2 UAABBColliderComponent::TransformVector2D(const Vector2& localPos, float angleRad, const Vector2& worldPos)
{
	Vector2 result;
	Vector2 local = localPos;

	result = local.Rotate(angleRad);

	// 최종 월드 위치 적용
	result.x += worldPos.x;
	result.y += worldPos.y;

	return result;
}
