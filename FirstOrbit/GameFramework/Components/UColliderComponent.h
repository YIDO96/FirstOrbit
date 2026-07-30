#pragma once

#include "UActorComponent.h"

enum class ColliderType
{
	AABB,
	OBB,
	Circle,
};

struct AABB
{
	Vector2 min;
	Vector2 max;
};

class AActor;

// 콜라이더 공용 베이스. 브로드페이즈용 AABB 보관, Begin/EndOverlap 이벤트, 도형 쌍별
// 정밀 충돌 판정 함수 모음을 담당한다. 실제 도형은 파생 클래스가 구현한다
// (UAABBColliderComponent = 회전 반영 박스, UCircleColliderComponent = 원).
class UColliderComponent : public UActorComponent
{
public:
	using OverlapDelegate = function<void(AActor*, AActor*)>;
	// Begin/Stay는 충돌 정보(HitResult: 법선/침투깊이)까지 함께 넘겨준다.
	// (공이 벽돌에 튕기는 것처럼, 어느 방향으로 밀어내야 하는지는 겹침 여부만으로는 알 수 없어서 필요하다)
	using HitOverlapDelegate = function<void(AActor*, AActor*, const HitResult&)>;

public:
	UColliderComponent(AActor* owner, ColliderType type) : UActorComponent(owner), _type(type) {}
	virtual ~UColliderComponent() = default;

	virtual void Init() override;
	// 전체 액터가 다 움직인 "다음"에 World가 호출하는 2단계 브로드페이즈 AABB 갱신.
	// 도형마다 계산 방식이 달라서 파생 클래스가 반드시 구현한다.
	virtual void UpdateAABB() = 0;
	virtual void Render(HDC hdc) override;

	Vector2 GetAbsolutePos();

	// 정밀 충돌 판정: this/other의 실제 도형 타입 조합에 맞는 아래 static 함수로 디스패치한다.
	// outHit.normal은 "this -> other" 방향으로 정규화되어 나온다.
	bool isCollisiion(AActor* other, HitResult& outHit);

	// 언리얼의 UPrimitiveComponent::BeginComponentOverlap / EndComponentOverlap과 동일한 역할.
	// 매 프레임 World가 액터 쌍마다 한 번씩 호출해서, 겹침 상태가 바뀌는 순간에만 이벤트를 쏜다.
	// (겹침이 유지되는 동안에는 매 프레임 StayOverlap이 갱신된 HitResult와 함께 호출된다)
	void CheckOverlap(AActor* other);
	void BeginOverlap(AActor* other, const HitResult& hit);
	void StayOverlap(AActor* other, const HitResult& hit);
	void EndOverlap(AActor* other);
	bool IsOverlapping(AActor* other) const { return _currentOverlap.find(other) != _currentOverlap.end(); }

	ColliderType GetColliderType() const { return _type; }
	Vector2 GetColliderSize() const { return _size; }
	float GetRadius() const { return _radius; }
	AABB GetAABB() const { return _aabb; }

	// ---- 도형 쌍별 정밀 충돌 판정 (순수 수학, static) ----
	// outHit이 nullptr가 아니면 "a -> b" 방향 법선과 침투 깊이를 채운다.
	static bool CheckAABBvsAABB(const AABB& a, const AABB& b, HitResult* outHit = nullptr);
	static bool CheckAABBvsCircle(const AABB& box, const Vector2& circleCenter, float circleRadius, HitResult* outHit = nullptr);
	static bool CheckCircleVsCircle(const Vector2& centerA, float radiusA, const Vector2& centerB, float radiusB, HitResult* outHit = nullptr);

public:
	// Init()에서 등록해두면 실제 겹침이 시작/유지/종료되는 순간에 호출된다. (self, other[, hit])
	vector<HitOverlapDelegate> OnComponentBeginOverlap;
	vector<HitOverlapDelegate> OnComponentStayOverlap;
	vector<OverlapDelegate> OnComponentEndOverlap;

protected:
	ColliderType	_type = ColliderType::AABB;

	Vector2			_absolutePos;
	Vector2			_relativePos;
	Vector2			_size;			// 박스 콜라이더가 사용
	float			_radius = 0.f;	// 원 콜라이더가 사용

	AABB			_aabb;			// 브로드페이즈용 바운딩 박스 (도형과 무관하게 World 그리드가 사용)

	unordered_set<AActor*> _currentOverlap;
};
