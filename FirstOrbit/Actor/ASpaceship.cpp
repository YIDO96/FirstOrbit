#include "pch.h"
#include "ASpaceship.h"

#include "Core/InputManager.h"
#include "Core/ResourceManager.h"
#include "GameFramework/Camera.h"
#include "GameFramework/Texture.h"
#include "GameFramework/World.h"
#include "GameFramework/Components/UPhysicsComponent.h"


void ASpaceship::Init()
{
	_size = Vector2(12.f, 20.f);
	_type = EActorType::Ship;
	_name = "SpaceShip";


	_physicsComp = AddComponent<UPhysicsComponent>();
	SetTexture(RESOURCE.GetTexture(L"Spaceship"));

}
void ASpaceship::Update(float deltaTime)
{
	Super::Update(deltaTime);

	Input(deltaTime);

	_trailSampleTimer += deltaTime;
	if (_trailSampleTimer >= 0.05f)
	{
		_trailSampleTimer = 0.f;
		_trail.push_back(GetCenterPos());
		if (_trail.size() > maxTrail)
			_trail.erase(_trail.begin());
	}
}
void ASpaceship::Render(HDC hdc)
{
	Super::Render(hdc);

	Camera& cam = _ownerWorld->GetCamera();

	if (_texture)
	{
		Vector2 screenPos = cam.WorldToScreen(GetCenterPos());
		Vector2 destSize(cam.WorldToScreenScale(_size.x), cam.WorldToScreenScale(_size.y));
		_texture->RenderRotated(hdc, screenPos, DegreeToRadian(GetDegree()), destSize);

	}

	// 기체를 자기 각도로 돌리고 → 월드 위치로 옮기고 → 뷰 변환
	//Matrix3x3 M = cam.GetViewMatrix()
	//	* Matrix3x3::Translate(GetCenterPos())
	//	* Matrix3x3::Rotate(DegreeToRadian(GetDegree()));


	//POINT pts[3];
	//for (int i = 0; i < 3; ++i)
	//{
	//	Vector2 p = M.TransformPoint(_localPoints[i]);
	//	pts[i].x = (long)p.x;
	//	pts[i].y = (long)p.y;
	//}
	//
	//HBRUSH brush = ::CreateSolidBrush(RGB(220, 220, 240));
	//HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);

	//::Polygon(hdc, pts, 3);
	//
	//::SelectObject(hdc, oldBrush);
	//::DeleteObject(brush);

	COLORREF color = RGB(255, 255, 0);
	if (_physicsComp)
	{
		_physicsComp->GetIntegrator() == EIntergrator::ExplicitEuler ? color = RGB(255, 0, 0) :
			_physicsComp->GetIntegrator() == EIntergrator::SemiImplicitEuler? color = RGB(0, 255, 0) :
			_physicsComp->GetIntegrator() == EIntergrator::RK4 ? color = RGB(0, 0, 255) : color = RGB(255, 255, 255);
	}


	for (const Vector2& p : _trail)
	{
		Vector2 s = cam.WorldToScreen(p);
		::SetPixel(hdc, (int)s.x, (int)s.y, color);
	}

}
void ASpaceship::OnGUI()
{
	const char* name = _name.c_str();

	if (ImGui::TreeNode(name))
	{
		Super::OnGUI();

		ImGui::SliderFloat("Thrust", &_thrust, 10.f, 60.f);
		ImGui::SliderFloat("RotSpeed", &_rotSpeed, 30.f, 180.f);


		ImGui::TreePop();
	}
}

void ASpaceship::Input(float deltaTime)
{
	if (_physicsComp->GetIsPaused()) return;   // 발사 대기 중엔 조작 자체를 막는다

	// Rotate (Right,Left)
	if (_INPUT.GetButtonPressed(KeyType::A) or _INPUT.GetButtonPressed(KeyType::Left)) AddRotation(-_rotSpeed * deltaTime);
	if (_INPUT.GetButtonPressed(KeyType::D) or _INPUT.GetButtonPressed(KeyType::Right)) AddRotation(_rotSpeed * deltaTime);
	
	_isThrusting = _INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up);
	
}

void ASpaceship::Reset()
{
	SetRotation(0.f);
	_physicsComp->Reset();
	_isThrusting = false;
	_keyInput = KeyType::L;
	_trail.clear();
	_trailSampleTimer = 0.f;
}
