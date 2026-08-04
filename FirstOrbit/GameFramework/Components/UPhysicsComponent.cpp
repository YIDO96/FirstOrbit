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
	if (_isPaused) return;
	//deltaTime *= 20.f;

	_accumulator += deltaTime;
	int steps = 0;
	while (_accumulator >= FIXED_DT && steps < MAX_STEPS)
	{
		PhysicsStep(FIXED_DT);
		_accumulator -= FIXED_DT;
		++steps;
	}
	if (steps >= MAX_STEPS) _accumulator = 0.f;   // 밀린 시간은 버린다
}

void UPhysicsComponent::Render(HDC hdc)
{
}

void UPhysicsComponent::OnGUI()
{
	Vector2 acceleration = ComputeAcceleration(GetOwner()->GetCenterPos(), _velocity);

	ImGui::SeparatorText("EIntergrator");
	if (ImGui::Button("ExplicitEuler", ImVec2(100, 0)))
	{
		_integrator = EIntergrator::ExplicitEuler;
	}
	ImGui::SameLine();
	if (ImGui::Button("SemiImplicitEuler", ImVec2(100, 0)))
	{
		_integrator = EIntergrator::SemiImplicitEuler;
	}
	ImGui::SameLine();
	if (ImGui::Button("RK4", ImVec2(100, 0)))
	{
		_integrator = EIntergrator::RK4;
	}
	ImGui::SeparatorText("Physics 상태");
	ImGui::Text("Pos X: %.2f, Y : %.2f", GetOwner()->GetCenterPos().x, GetOwner()->GetCenterPos().y);
	ImGui::Text("Vel X: %.2f, Y : %.2f", _velocity.x, _velocity.y);
	ImGui::Text("Speed: %.2f", _velocity.Length());
	ImGui::Text("Acceleration X: %.2f, Y : %.2f", acceleration.x, acceleration.y);
	ImGui::Text("Length: %.2f", acceleration.Length());
	ImGui::Text("Height: %.2f", GetOwner()->GetCenterPos().y);


	ASpaceship* ship = (GetOwner()->GetType() == EActorType::Ship) 
		? static_cast<ASpaceship*>(GetOwner()) : nullptr;
	APlanet* targetPlanet = ship ? ship->GetTargetPlanet() : nullptr;
	
	if (targetPlanet)
	{
		float r = (GetOwner()->GetCenterPos() - targetPlanet->GetCenterPos()).Length();
		float E = 0.5f * _velocity.LengthSquared() - targetPlanet->GetMu() / r;
		ImGui::Text("r: %.4f", r);
		ImGui::Text("Energy: %.2f", E);
	}
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

void UPhysicsComponent::PhysicsStep(float deltaTime)
{
	if (_integrator == EIntergrator::ExplicitEuler)
	{
		// 명시적 오일러
		Vector2 oldPos = GetOwner()->GetCenterPos();
		Vector2 oldVel = _velocity;


		Vector2 newPos = oldPos + oldVel * deltaTime;
		Vector2 newVel = oldVel + ComputeAcceleration(oldPos, oldVel) * deltaTime;
		GetOwner()->SetCenterPos(newPos);
		_velocity = newVel;
	}
	else if (_integrator == EIntergrator::SemiImplicitEuler)
	{
		// 반암시적 오일러
		_velocity += ComputeAcceleration(GetOwner()->GetCenterPos(), _velocity) * deltaTime;
		GetOwner()->SetCenterPos(GetOwner()->GetCenterPos() + _velocity * deltaTime);
	}
	else if (_integrator == EIntergrator::RK4)
	{
		// RK4
		Vector2 x0 = GetOwner()->GetCenterPos();
		Vector2 v0 = _velocity;

		// K1
		Vector2 a1 = ComputeAcceleration(x0, v0);
		Vector2 k1_x = v0;			// 시작점의 속도
		Vector2 k1_v = a1;			// 시작점의 가속도


		// K2
		Vector2 x_k2 = x0 + k1_x * (deltaTime * 0.5f);
		Vector2 v_k2 = v0 + k1_v * (deltaTime * 0.5f);
		Vector2 a2 = ComputeAcceleration(x_k2, v_k2);
		Vector2 k2_x = v_k2;			// K2에서의 속도
		Vector2 k2_v = a2;			// K2에서의 가속도


		// K3
		Vector2 x_k3 = x0 + k2_x * (deltaTime * 0.5f);
		Vector2 v_k3 = v0 + k2_v * (deltaTime * 0.5f);
		Vector2 a3 = ComputeAcceleration(x_k3, v_k3);
		Vector2 k3_x = v_k3;			// K3에서의 속도
		Vector2 k3_v = a3;			// K3에서의 가속도

		// K4
		Vector2 x_k4 = x0 + k3_x * deltaTime;
		Vector2 v_k4 = v0 + k3_v * deltaTime;
		Vector2 a4 = ComputeAcceleration(x_k4, v_k4);
		Vector2 k4_x = v_k4;			// K4에서의 속도
		Vector2 k4_v = a4;			// K4에서의 가속도

		Vector2 newPos = x0 + (k1_x + 2.f * k2_x + 2.f * k3_x + k4_x) * (deltaTime / 6.f);
		Vector2 newVel = v0 + (k1_v + 2.f * k2_v + 2.f * k3_v + k4_v) * (deltaTime / 6.f);

		GetOwner()->SetCenterPos(newPos);
		_velocity = newVel;


	}
}
