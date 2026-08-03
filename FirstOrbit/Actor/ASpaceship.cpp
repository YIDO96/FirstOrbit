#include "pch.h"
#include "ASpaceship.h"

#include "Core/InputManager.h"
#include "GameFramework/Camera.h"
#include "GameFramework/World.h"
#include "GameFramework/Components/UPhysicsComponent.h"


void ASpaceship::Init()
{
	_size = Vector2(12.f, 20.f);
	_type = EActorType::Ship;
	_name = "SpaceShip";


	UPhysicsComponent* physicsComp = AddComponent<UPhysicsComponent>();


}
void ASpaceship::Update(float deltaTime)
{
	Super::Update(deltaTime);

	Input(deltaTime);

}
void ASpaceship::Render(HDC hdc)
{
	Super::Render(hdc);

	Camera& cam = _ownerWorld->GetCamera();

	// 기체를 자기 각도로 돌리고 → 월드 위치로 옮기고 → 뷰 변환
	Matrix3x3 M = cam.GetViewMatrix()
		* Matrix3x3::Translate(GetCenterPos())
		* Matrix3x3::Rotate(DegreeToRadian(GetDegree()));


	POINT pts[3];
	for (int i = 0; i < 3; ++i)
	{
		Vector2 p = M.TransformPoint(_localPoints[i]);
		pts[i].x = (long)p.x;
		pts[i].y = (long)p.y;
	}

	HBRUSH brush = ::CreateSolidBrush(RGB(220, 220, 240));
	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);

	::Polygon(hdc, pts, 3);

	::SelectObject(hdc, oldBrush);
	::DeleteObject(brush);
}
void ASpaceship::OnGUI()
{
	const char* name = _name.c_str();

	if (ImGui::TreeNode(name))
	{
		Super::OnGUI();





		ImGui::TreePop();
	}
}

void ASpaceship::Input(float deltaTime)
{
	// Rotate (Right,Left)
	if (_INPUT.GetButtonPressed(KeyType::A)or _INPUT.GetButtonPressed(KeyType::Left)) AddRotation(-_rotSpeed * deltaTime);
	if (_INPUT.GetButtonPressed(KeyType::D)or _INPUT.GetButtonPressed(KeyType::Right)) AddRotation(_rotSpeed * deltaTime);

	// Move (Up,Down)
	//if (_INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up)) _pos += _forwardDir * _moveSpeed * deltaTime;
	_isThrusting = _INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up);
}
