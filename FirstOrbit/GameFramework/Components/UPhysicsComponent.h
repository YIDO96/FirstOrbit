#pragma once

#include "GameFramework/Components/UActorComponent.h"

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


	Vector2 ComputeAcceleration(const Vector2& pos, const Vector2& vel) const;

private:
	// 가속도는 따로 두지 않음 (실시간 계산)
	Vector2 _velocity = Vector2(0.f, 0.f);
	float _mass = 1.f;
};

