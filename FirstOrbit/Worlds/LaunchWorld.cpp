#include "pch.h"
#include "LaunchWorld.h"

#include "Core/GameInstance.h"
#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/UIManager.h"

#include "Actor/ASpaceship.h"
#include "Actor/StarField.h"

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
	_launchWidget->BindShip(_ship);

	_stars = new StarField();
	_stars->Init(3000, 6000.f);
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

	if (launchGameMode->GetLaunchState() == ELaunchState::Ascent)
	{
		_altitude = -_ship->GetCenterPos().y;   // 위가 -y
		if (_altitude > 5000.f)
		{
			LaunchHandoff handoff;
			handoff.position = ((_ship->GetCenterPos()* 0.2f) - Vector2(0, 0)).Normalized() * 1000.f;
			//handoff.position = _ship->GetCenterPos();
			handoff.velocity = _ship->GetComponent<UPhysicsComponent>()->GetVelocity();
			handoff.degree = _ship->GetDegree();
			GAME.SetLaunchHandoff(handoff);

			WORLD.ChangeWorld("MainWorld");
		}

		float density = expf(-_altitude / GAtmosphereScaleHeight);
		_camera.SetZoom(.3f + (2.f - .3f) * density);
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

	

	_launchWidget->SetFuelRatio(_ship->GetFuel() / _ship->GetMaxFuel());
}
void LaunchWorld::Render(HDC hdc)
{
	// TODO(대기권 연출):
	// 1) altitude 0 → 1000(핸드오프 고도) 사이를 0~1로 정규화
	float normalizedAltitude = Normalize(_altitude, 0.f, 1000.f);
	// 2) 하늘색을 파랑(RGB(135,206,235) 같은) → 검정으로 보간해서
	//    FillRect(hdc, 화면전체Rect, 그색으로만든브러시) 로 배경을 채우기
	//    (Super::Render(hdc)보다 먼저 그려야 그 위에 행성/우주선이 올라감)

	float density = expf(-_altitude / GAtmosphereScaleHeight);
	RECT back{ 0, 0, GWinSizeX, GWinSizeY };
	//COLORREF backColor = LerpColor(RGB(135, 206, 235), RGB(0, 0, 0), normalizedAltitude);
	COLORREF backColor = ComputeRayleighSkyColor(density);
	HBRUSH backBrush = ::CreateSolidBrush(backColor);
	::FillRect(hdc, &back, backBrush);
	::DeleteObject(backBrush);

	_stars->Render(hdc, _camera, 1.f - density);

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
COLORREF LaunchWorld::ComputeRayleighSkyColor(float density)
{
	// 실측 레일리 산란계수 비율 (β ∝ 1/λ⁴, R≈650nm G≈530nm B≈470nm 기준)
	static const float betaR[3] = { 0.27f, 0.61f, 1.0f };

	//float density = expf(-altitude / kScaleHeight);   // 대기 밀도: 고도 0에서 1, 우주로 갈수록 0

	// TODO: k는 노출값(하늘이 얼마나 밝게 보일지) — 255 근처에서 시작해서 취향껏 조정
	float k = 255.f;

	BYTE r = (BYTE)clamp(k * density * betaR[0], 0.f, 255.f);
	BYTE g = (BYTE)clamp(k * density * betaR[1], 0.f, 255.f);
	BYTE b = (BYTE)clamp(k * density * betaR[2], 0.f, 255.f);

	return RGB(r, g, b);
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
