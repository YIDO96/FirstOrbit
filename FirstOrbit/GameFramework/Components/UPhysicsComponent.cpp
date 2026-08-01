#include "pch.h"
#include "UPhysicsComponent.h"

#include "GameFramework/AActor.h"

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
	ImGui::SeparatorText("상태");
	ImGui::Text("Pos X: %.2f, Y : %.2f", GetOwner()->GetCenterPos().x, GetOwner()->GetCenterPos().y);
	ImGui::Text("Vel X: %.2f, Y : %.2f", _velocity.x, _velocity.y);
	ImGui::Text("Speed: %.2f", _velocity.Length());
	ImGui::Text("Acceleration X: %.2f, Y : %.2f", ComputeAcceleration(GetOwner()->GetCenterPos(), _velocity).x, ComputeAcceleration(GetOwner()->GetCenterPos(), _velocity).y);
	ImGui::Text("Length: %.2f", ComputeAcceleration(GetOwner()->GetCenterPos(), _velocity).Length());
	ImGui::Text("Height: %.2f", GetOwner()->GetCenterPos().y);
}

Vector2 UPhysicsComponent::ComputeAcceleration(const Vector2& pos, const Vector2& vel) const
{
	return Vector2(0, 20);
}
