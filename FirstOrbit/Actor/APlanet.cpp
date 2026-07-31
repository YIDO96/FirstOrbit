#include "pch.h"
#include "APlanet.h"

#include "GameFramework/Camera.h"
#include "GameFramework/World.h"
#include "GameFramework/Texture.h"

#include "GameFramework/Components/UCircleColliderComponent.h"

void APlanet::Init()
{
	_circleCollider = AddComponent<UCircleColliderComponent>();
	_collider = _circleCollider;
	
	//collider->Init(this, BALL_SIZE / 2);
	//collider->SetCheckCell(true);
}
void APlanet::Update(float deltaTime)
{
	_angle += _orbitSpeed * deltaTime;
	Vector2 p = _orbitCenter + Vector2(cosf(_angle), sinf(_angle)) * _orbitRadius;
	SetCenterPos(p);

	Super::Update(deltaTime);
}
void APlanet::Render(HDC hdc)
{
	Super::Render(hdc);

	Camera& cam = _ownerWorld->GetCamera();
	Vector2 screenPos = cam.WorldToScreen(GetCenterPos());
	float r = cam.WorldToScreenScale(_bodyRadius);

	if (_texture)
	{
		Vector2 destSize(max(r * 2.f, 1.f), max(r * 2.f, 1.f)); // 0px 방지
		Vector2 renderPos = screenPos - destSize / 2.f;

		_texture->Render(hdc, renderPos, Vector2(), Vector2(), destSize);

	}


	//if (r >= 30.f)
	//{
	//	// 원 + 표면 디테일 (일단은 테두리만)
	//	HBRUSH brush = ::CreateSolidBrush(RGB(100, 150, 255));
	//	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);
	//	HPEN pen = ::CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
	//	HPEN oldPen = (HPEN)::SelectObject(hdc, pen);
	//
	//	::Ellipse(hdc,
	//		(int)(screenPos.x - r), (int)(screenPos.y - r),
	//		(int)(screenPos.x + r), (int)(screenPos.y + r));
	//
	//	::SelectObject(hdc, oldBrush);
	//	::SelectObject(hdc, oldPen);
	//	::DeleteObject(brush);
	//	::DeleteObject(pen);
	//}
	//else if (r >= 3.f)
	//{
	//	// 단색 원
	//	HBRUSH brush = ::CreateSolidBrush(RGB(100, 150, 255));
	//	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);
	//
	//	::Ellipse(hdc,
	//		(int)(screenPos.x - r), (int)(screenPos.y - r),
	//		(int)(screenPos.x + r), (int)(screenPos.y + r));
	//
	//	::SelectObject(hdc, oldBrush);
	//	::DeleteObject(brush);
	//}
	//else
	//{
	//	// SetPixel
	//	::SetPixel(hdc, (int)screenPos.x, (int)screenPos.y, RGB(255, 255, 255));
	//}
}
void APlanet::OnGUI()
{
	const char* name = _name.c_str();

	if (ImGui::TreeNode(name))
	{

		ImGui::TreePop();
	}
}

void APlanet::Setup(string name, Vector2 orbitCenter, float orbitRadius, float orbitSpeed, float mass, float bodyRadius, float initialAngle)
{
	_name = name;
	_orbitCenter = orbitCenter;
	_orbitRadius = orbitRadius;
	_orbitSpeed = orbitSpeed;
	_mass = mass;
	_bodyRadius = bodyRadius;
	_angle = initialAngle;

	_size = Vector2(_bodyRadius * 2, _bodyRadius * 2);

	// 
	Vector2 center = _orbitCenter + Vector2(cosf(_angle), sinf(_angle)) * orbitRadius;
	SetCenterPos(center);


	_circleCollider->SetRadius(_bodyRadius);
}