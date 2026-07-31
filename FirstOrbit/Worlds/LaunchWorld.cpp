#include "pch.h"
#include "LaunchWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"

#include "Actor/ASpaceship.h"

void LaunchWorld::Enter()
{
	Super::Enter();


	_camera.SetIsControll(true);
	_camera.SetZoomImmediate(4.0f);
	_camera.SetPosition(Vector2(0, 0));

	_ship = SpawnActor<ASpaceship>();
	_ship->SetCenterPos(Vector2(0.f, -10.f));
}
void LaunchWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (_INPUT.GetButtonDown(KeyType::SpaceBar))
		WORLD.ChangeWorld("MainWorld");
}
void LaunchWorld::Render(HDC hdc)
{
	Super::Render(hdc);

	// 바닥선 (월드 y=0, 좌우로 길게)
	Vector2 left = _camera.WorldToScreen(Vector2(-5000.f, 0.f));
	Vector2 right = _camera.WorldToScreen(Vector2(5000.f, 0.f));

	HPEN pen = ::CreatePen(PS_SOLID, 2, RGB(120, 200, 120));
	HPEN oldPen = (HPEN)::SelectObject(hdc, pen);

	::MoveToEx(hdc, (int)left.x, (int)left.y, nullptr);
	::LineTo(hdc, (int)right.x, (int)right.y);

	::SelectObject(hdc, oldPen);
	::DeleteObject(pen);
}
void LaunchWorld::LoadTexture()
{
	
}
void LaunchWorld::OnSceneGUI()
{
	ImGui::Text("SpaceBar : MainWorld로");
}