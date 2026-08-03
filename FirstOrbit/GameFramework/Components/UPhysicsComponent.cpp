#include "pch.h"
#include "UPhysicsComponent.h"

#include "GameFramework/AActor.h"
#include "GameFramework/World.h"

#include "Actor/ASpaceship.h"
#include "Actor/APlanet.h"

void UPhysicsComponent::Init()
{
}

void UPhysicsComponent::Update(float deltaTime)
{
	_velocity += ComputeAcceleration(GetOwner()->GetCenterPos(), _velocity) * deltaTime;
	GetOwner()->SetCenterPos(GetOwner()->GetCenterPos() + _velocity * deltaTime);
}

void UPhysicsComponent::Render(HDC hdc)
{
}

void UPhysicsComponent::OnGUI()
{
	Vector2 acceleration = ComputeAcceleration(GetOwner()->GetCenterPos(), _velocity);

	ImGui::SeparatorText("상태");
	ImGui::Text("Pos X: %.2f, Y : %.2f", GetOwner()->GetCenterPos().x, GetOwner()->GetCenterPos().y);
	ImGui::Text("Vel X: %.2f, Y : %.2f", _velocity.x, _velocity.y);
	ImGui::Text("Speed: %.2f", _velocity.Length());
	ImGui::Text("Acceleration X: %.2f, Y : %.2f", acceleration.x, acceleration.y);
	ImGui::Text("Length: %.2f", acceleration.Length());
	ImGui::Text("Height: %.2f", GetOwner()->GetCenterPos().y);

	//if (targetPlanet)
	//{
	//	float r = (GetOwner()->GetCenterPos() - targetPlanet->GetCenterPos()).Length();
	//	float E = 0.5f * _velocity.LengthSquared() - targetPlanet->GetMu() / r;
	//	ImGui::Text("Energy: %.2f", E);
	//}
}


// 로켓 가속도 계산 함수
Vector2 UPhysicsComponent::ComputeAcceleration(const Vector2& pos, const Vector2& vel) const
{
	Vector2 a;

	ASpaceship* ship = nullptr;
	if (GetOwner()->GetType() == EActorType::Ship)
	{
		ship = static_cast<ASpaceship*>(GetOwner());
	}

	APlanet* targetPlanet = ship ? ship->GetTargetPlanet() : nullptr;

	if (targetPlanet)
	{
		Vector2 dir = targetPlanet->GetCenterPos() - pos;
		float r2 = dir.LengthSquared();
		float r = sqrtf(r2);

		r = max(r, targetPlanet->GetBodyRadius());  // r -> 0 클램프 (Nan 방지)
		r2 = r * r;

		a += dir * (targetPlanet->GetMu() / (r2 * r));
	}
	else
	{
		a += Vector2(0.f, 20.f);   // LaunchWorld: 균일 중력
	}


	if (ship != nullptr and ship->GetIsThrusting())
	{
		a += ship->GetForwardDir() * (ship->GetThrust() / _mass);
	}

	return a;

}
