#pragma once

#include "GameFramework/Components/UActorComponent.h"


enum class EIntergrator
{
	ExplicitEuler,		// 기본 오일러 방법
	SemiImplicitEuler,		// 심플렉틱 오일러
	RK4						// 4차 룬게-쿠타
};

class UPhysicsComponent : public UActorComponent
{
	using Super = UActorComponent;

public:
	UPhysicsComponent(AActor* owner) : Super(owner) {}
	virtual ~UPhysicsComponent() = default;

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;

	void Reset() { _velocity = Vector2(0.f, 0.f); _accumulator = 0.f; }


	Vector2 ComputeAcceleration(const Vector2& pos, const Vector2& vel) const;
	EIntergrator GetIntegrator() const { return _integrator; }

	void SetPaused(bool isPause) { _isPaused = isPause; }
	void SetVelocity(const Vector2& velocity) { _velocity = velocity; }
	Vector2 GetVelocity() const { return _velocity; }
	bool GetIsPaused() const { return _isPaused; }

private:
	void PhysicsStep(float deltaTime);

private:
	// 가속도는 따로 두지 않음 (실시간 계산)
	Vector2 _velocity = Vector2(0.f, 0.f);
	float _mass = 1.f;

	EIntergrator _integrator = EIntergrator::RK4;

	// 고정 시간 스텝 시뮬레이션
	float _accumulator = 0.f;
	static constexpr float FIXED_DT = 1.f / 240.f;
	static constexpr int MAX_STEPS = 5;

	bool _isPaused = true;
};

