#include "pch.h"
#include "LaunchWorld.h"

#include "Core/GameInstance.h"
#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/UIManager.h"

#include "Actor/ASpaceship.h"

#include "GameFramework/GameMode.h"
#include "GameFramework/Components/UPhysicsComponent.h"
#include "GameMode/LaunchGameMode.h"

#include "Widget/Widget_Launch.h"

void LaunchWorld::Enter()
{
	Super::Enter();


	_camera.SetIsControll(true);
	_camera.SetZoomImmediate(2.0f);
	

	_ship = SpawnActor<ASpaceship>();
	_ship->SetCenterPos(Vector2(0.f, -10.f));
	GetGameMode<LaunchGameMode>()->SetShip(_ship);

	_camera.SetPosition(_ship->GetCenterPos() + Vector2(0, -100.f));

	_launchWidget = UI.CreateWidget<Widget_Launch>();
	_launchWidget->SetOwnerWorld(this);
}
void LaunchWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	

	LaunchGameMode* launchGameMode = GetGameMode<LaunchGameMode>();
	//if (launchGameMode->GetLaunchState() == ELaunchState::Idle and
	//	_INPUT.GetButtonDown(KeyType::SpaceBar))
	//{
	//	launchGameMode->ChangeLaunchState(ELaunchState::Countdown);
	//}

	//if (launchGameMode->GetLaunchState() == ELaunchState::Ascent)
	{
		float altitude = -_ship->GetCenterPos().y;   // 위가 -y
		if (altitude > 1000.f)
		{
			LaunchHandoff handoff;
			handoff.position = (_ship->GetCenterPos() - Vector2(0, 0)).Normalized() * 1000.f;
			//handoff.position = _ship->GetCenterPos();
			handoff.velocity = _ship->GetComponent<UPhysicsComponent>()->GetVelocity();
			handoff.degree = _ship->GetDegree();
			GAME.SetLaunchHandoff(handoff);

			WORLD.ChangeWorld("MainWorld");
		}
	}

	
	if (_INPUT.GetButtonDown(KeyType::R))
	{
		launchGameMode->Reset();
		_ship->SetCenterPos(Vector2(0.f, -10.f));   // 발사대 위치는 여전히 LaunchWorld 책임
		_camera.SetPosition(_ship->GetCenterPos() + Vector2(0, -100.f));
		_camera.SetZoomImmediate(2.f);

		_launchWidget->ResetWidget();
	}

	if (_INPUT.GetButtonDown(KeyType::P))
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
	ImGui::Text("P : MainWorld로");
	Vector2 v = _ship->GetComponent<UPhysicsComponent>()->GetVelocity();
	ImGui::Text("수평속도: %.1f / 목표 ~100", v.x);
}

GameMode* LaunchWorld::CreateGameMode()
{
	return new LaunchGameMode();
}
