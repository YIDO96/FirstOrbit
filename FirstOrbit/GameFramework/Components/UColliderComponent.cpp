#include "pch.h"
#include "UColliderComponent.h"

#include "GameFramework/AActor.h"
#include "Core/GameInstance.h"

void UColliderComponent::Init()
{
	_size = _owner->GetSize();
}

void UColliderComponent::Render(HDC hdc)
{
	// F1로 토글되는 디버그 모드에서만 콜라이더 히트박스를 그린다.
	if (!GameInstance::GetInstance().IsDebugMode())
		return;

	// 1. 선의 색상과 두께를 지정할 펜(Pen) 생성
	HPEN greenPen = ::CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	HPEN oldPen = (HPEN)::SelectObject(hdc, greenPen);

	// 2. 속을 채우지 않는 투명 브러시(HOLLOW_BRUSH) 가져오기
	HBRUSH transparentBrush = (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, transparentBrush);

	// float를 int로 그냥 대입하면 소수점이 버림(truncate)되어 픽셀이 한쪽으로 쏠릴 수 있으므로 반올림한다.
	Rectangle(hdc,
		static_cast<int32>(std::lround(_aabb.min.x)),
		static_cast<int32>(std::lround(_aabb.min.y)),
		static_cast<int32>(std::lround(_aabb.max.x)),
		static_cast<int32>(std::lround(_aabb.max.y)));

	// 4. 사용이 끝난 GDI 오브젝트들은 반드시 되돌려놓고 반납(메모리 누수 방지)
	::SelectObject(hdc, oldPen);
	::SelectObject(hdc, oldBrush);
	::DeleteObject(greenPen); // StockObject는 Delete하지 않아도 되지만 직접 만든 Pen은 지워야 합니다.
}

Vector2 UColliderComponent::GetAbsolutePos()
{
	if (_owner)
	{
		// 부모 위치 + 나의 상대 위치 = 나의 진짜 위치 (언리얼의 Attach 관계와 동일한 원리)
		return _owner->GetCenterPos() + _relativePos;
	}
	return _relativePos;
}

bool UColliderComponent::isCollisiion(AActor* other, HitResult& outHit)
{
	UColliderComponent* otherCol = other->GetCollider();
	if (!otherCol) return false;

	bool aIsCircle = (_type == ColliderType::Circle);
	bool bIsCircle = (otherCol->GetColliderType() == ColliderType::Circle);

	if (aIsCircle && bIsCircle)
	{
		return CheckCircleVsCircle(
			_owner->GetCenterPos(), _radius,
			otherCol->GetOwner()->GetCenterPos(), otherCol->GetRadius(), &outHit);
	}
	else if (aIsCircle && !bIsCircle)
	{
		// CheckAABBvsCircle은 "박스 -> 원" 방향 법선을 주는데, 여기선 self(원) -> other(박스)
		// 기준이 필요하므로 뒤집는다.
		bool hit = CheckAABBvsCircle(otherCol->GetAABB(), _owner->GetCenterPos(), _radius, &outHit);
		outHit.normal = -outHit.normal;
		return hit;
	}
	else if (!aIsCircle && bIsCircle)
	{
		return CheckAABBvsCircle(_aabb, otherCol->GetOwner()->GetCenterPos(), otherCol->GetRadius(), &outHit);
	}
	else
	{
		return CheckAABBvsAABB(_aabb, otherCol->GetAABB(), &outHit);
	}
}

bool UColliderComponent::CheckAABBvsAABB(const AABB& a, const AABB& b, HitResult* outHit)
{
	if (a.max.x < b.min.x) return false; // a가 b보다 완전히 왼쪽
	if (a.min.x > b.max.x) return false; // a가 b보다 완전히 오른쪽
	if (a.max.y < b.min.y) return false; // a가 b보다 완전히 위쪽
	if (a.min.y > b.max.y) return false; // a가 b보다 완전히 아래쪽

	if (outHit)
	{
		// 겹친 폭(overlapX)/높이(overlapY) 중 더 얕은 쪽을 밀어내는 축으로 삼는다 (MTV).
		// std::min/max는 windows.h의 min/max 매크로와 충돌하므로(이 프로젝트 관례상) 괄호로 감싼 매크로 버전을 쓴다.
		float overlapX = min(a.max.x, b.max.x) - max(a.min.x, b.min.x);
		float overlapY = min(a.max.y, b.max.y) - max(a.min.y, b.min.y);

		Vector2 centerA((a.min.x + a.max.x) * 0.5f, (a.min.y + a.max.y) * 0.5f);
		Vector2 centerB((b.min.x + b.max.x) * 0.5f, (b.min.y + b.max.y) * 0.5f);

		if (overlapX < overlapY)
		{
			outHit->depth = overlapX;
			outHit->normal = Vector2((centerB.x >= centerA.x) ? 1.f : -1.f, 0.f);
		}
		else
		{
			outHit->depth = overlapY;
			outHit->normal = Vector2(0.f, (centerB.y >= centerA.y) ? 1.f : -1.f);
		}
	}

	return true;
}

bool UColliderComponent::CheckAABBvsCircle(const AABB& box, const Vector2& circleCenter, float circleRadius, HitResult* outHit)
{
	// 박스 내부에서 원의 중심과 가장 가까운 점을 찾고, 그 점까지의 거리가 반지름 이내면 충돌이다.
	float closestX = std::clamp(circleCenter.x, box.min.x, box.max.x);
	float closestY = std::clamp(circleCenter.y, box.min.y, box.max.y);

	float dx = circleCenter.x - closestX;
	float dy = circleCenter.y - closestY;
	float distSquared = dx * dx + dy * dy;

	if (distSquared > circleRadius * circleRadius)
		return false;

	if (outHit)
	{
		float distance = ::sqrtf(distSquared);

		if (distance < SMALL_NUMBER)
		{
			// 원의 중심이 박스 안에 파묻힌 경우: closest == center라 dx/dy로는 방향을 못 구한다.
			// 4개 면 중 가장 얕게 겹친 쪽으로 밀어낸다.
			float left = circleCenter.x - box.min.x;
			float right = box.max.x - circleCenter.x;
			float top = circleCenter.y - box.min.y;
			float bottom = box.max.y - circleCenter.y;

			float minOverlap = min(min(left, right), min(top, bottom));

			if (minOverlap == left)		outHit->normal = Vector2(-1.f, 0.f);
			else if (minOverlap == right)	outHit->normal = Vector2(1.f, 0.f);
			else if (minOverlap == top)	outHit->normal = Vector2(0.f, -1.f);
			else							outHit->normal = Vector2(0.f, 1.f);

			outHit->depth = minOverlap + circleRadius;
		}
		else
		{
			outHit->normal = Vector2(dx / distance, dy / distance);
			outHit->depth = circleRadius - distance;
		}
	}

	return true;
}

bool UColliderComponent::CheckCircleVsCircle(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, HitResult* outHit)
{
	float dx = centerB.x - centerA.x;
	float dy = centerB.y - centerA.y;
	float radiusSum = radiusA + radiusB;
	float distSquared = dx * dx + dy * dy;

	if (distSquared > radiusSum * radiusSum)
		return false;

	if (outHit)
	{
		float distance = ::sqrtf(distSquared);

		if (distance < SMALL_NUMBER)
			outHit->normal = Vector2(1.f, 0.f); // 완전히 겹친 경우 임의 방향
		else
			outHit->normal = Vector2(dx / distance, dy / distance);

		outHit->depth = radiusSum - distance;
	}

	return true;
}

void UColliderComponent::CheckOverlap(AActor* other)
{
	if (!other) return;

	UColliderComponent* otherCol = other->GetCollider();
	if (!otherCol) return;

	HitResult hit;
	bool isNowOverlap = isCollisiion(other, hit);
	bool wasOverlap = IsOverlapping(other);

	// otherCol 입장에서는 법선이 반대(other -> self) 방향이어야 한다.
	HitResult hitFromOther{ -hit.normal, hit.depth };

	if (isNowOverlap && !wasOverlap)
	{
		BeginOverlap(other, hit);
		otherCol->BeginOverlap(_owner, hitFromOther);
	}
	else if (isNowOverlap && wasOverlap)
	{
		StayOverlap(other, hit);
		otherCol->StayOverlap(_owner, hitFromOther);
	}
	else if (!isNowOverlap && wasOverlap)
	{
		EndOverlap(other);
		otherCol->EndOverlap(_owner);
	}
}

void UColliderComponent::BeginOverlap(AActor* other, const HitResult& hit)
{
	_currentOverlap.insert(other);

	for (auto& callback : OnComponentBeginOverlap)
		callback(_owner, other, hit);
}

void UColliderComponent::StayOverlap(AActor* other, const HitResult& hit)
{
	for (auto& callback : OnComponentStayOverlap)
		callback(_owner, other, hit);
}

void UColliderComponent::EndOverlap(AActor* other)
{
	_currentOverlap.erase(other);

	for (auto& callback : OnComponentEndOverlap)
		callback(_owner, other);
}
