#include "pch.h"
#include "LaunchWorld.h"

#include "Core/GameInstance.h"
#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/UIManager.h"
#include "Core/SoundManager.h"

#include "Actor/ASpaceship.h"
#include "Actor/StarField.h"

#include "GameFramework/GameMode.h"
#include "GameFramework/Components/UPhysicsComponent.h"
#include "GameMode/LaunchGameMode.h"

#include "GameSystem/SpaceLightingSystem.h"

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
	//_camera.SetFollowTarget(_ship);

	_launchWidget = UI.CreateWidget<Widget_Launch>();
	_launchWidget->SetOwnerWorld(this);
	_launchWidget->BindShip(_ship);

	_stars = new StarField();
	_stars->Init(3000, 6000.f);
	

	for (int cluster = 0; cluster < 10; ++cluster)
	{
		float clusterX = (float)(rand() % 1200) - 600.f;         // -400 ~ 400 (발사대 좌우)
		float clusterAltitude = 300.f + (float)(rand() % 700);   // 고도 300 ~ 900 구간에 배치
		Vector2 clusterPos(clusterX, -clusterAltitude);          // 위가 -y
		_cloudDriftSpeed = (rand() % 2 == 0 ? 1.f : -1.f) * (10.f + (float)(rand() % 20));

		int puffCount = 4 + rand() % 4;
		for (int i = 0; i < puffCount; ++i)
		{
			CloudPuff p;
			p.pos = clusterPos + Vector2((float)(rand() % 160 - 80), (float)(rand() % 60 - 30));
			p.radius = 60.f + (float)(rand() % 60);
			_clouds.push_back(p);
		}
	}

	_lightingSystem = new SpaceLightingSystem();
	HDC hdc = ::GetDC(GAME.GetHwnd());
	_lightingSystem->Initialize(hdc, GWinSizeX, GWinSizeY);
	::ReleaseDC(GAME.GetHwnd(), hdc);
}
void LaunchWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);
	_lastDeltaTime = deltaTime;

	for (CloudPuff& cloud : _clouds)
		cloud.pos.x += _cloudDriftSpeed * deltaTime;

	for (auto it = _launchSmoke.begin(); it != _launchSmoke.end(); )
	{
		it->currentLife -= deltaTime;
		if (it->currentLife <= 0.f) { it = _launchSmoke.erase(it); continue; }

		it->pos += it->velocity * deltaTime;
		it->velocity *= powf(0.3f, deltaTime);   // 초당 30%로 감속 (관성으로 퍼지다 서서히 멈춤)

		++it;
	}

	if (not _cameraFollowStarted)
	{
		Vector2 shipScreenPos = _camera.WorldToScreen(_ship->GetCenterPos());
		if (shipScreenPos.y <= GWinSizeY / 3.f)   // 화면상 1/3 지점에 도달
		{
			_cameraFollowStarted = true;
			_camera.SetFollowTarget(_ship);
		}
	}
	else
	{
		float offsetY = GWinSizeY / 6.f / _camera.GetZoom();
		_camera.SetFollowOffset(Vector2(0.f, offsetY));
	}
	

	LaunchGameMode* launchGameMode = GetGameMode<LaunchGameMode>();

	if (launchGameMode->GetLaunchState() == ELaunchState::Ascent)
	{
		if (not _isPlayLaunchSound)
		{
			SOUND.Play(L"S_SpaceshipLaunch");
			_isPlayLaunchSound = true;

			Vector2 padPos(0.f, -10.f);   // Enter()의 우주선 초기 위치 = 발사대 지면
			for (int i = 0; i < 40; ++i)
			{
				SmokePuff p;
				float side = (rand() % 2 == 0 ? 1.f : -1.f);
				p.pos = padPos + Vector2((float)(rand() % 80 - 40), (float)(rand() % 20));
				p.velocity = Vector2(side * (30.f + rand() % 70), 10.f + rand() % 20);   // 좌우로 퍼지며 살짝만 위로
				p.radius = 50.f + rand() % 50;
				p.maxLife = 4.f + (rand() % 300) / 100.f;   // 4~7초, 꽤 오래 남아있음
				p.currentLife = p.maxLife;
				_launchSmoke.push_back(p);
			}
		}

		_altitude = -_ship->GetCenterPos().y;   // 위가 -y
		if (_altitude > 2500.f)
		{
			SOUND.Stop(L"S_SpaceshipLaunch");
			LaunchHandoff handoff;
			handoff.position = ((_ship->GetCenterPos()* 0.4f) - Vector2(0, 0)).Normalized() * 1000.f;
			//handoff.position = _ship->GetCenterPos();
			handoff.velocity = _ship->GetComponent<UPhysicsComponent>()->GetVelocity();
			handoff.degree = _ship->GetDegree();
			handoff.autoOrbit = _ship->IsAutoPilot();
			GAME.SetLaunchHandoff(handoff);

			WORLD.ChangeWorld("MainWorld");
		}

		float density = expf(-_altitude / GAtmosphereScaleHeight);
		_camera.SetZoom(.3f + (2.f - .3f) * density);

		if (_ship->GetIsThrusting())
		{
			_camera.AddShake(0.1f, 1.f * density); // 저고도일수록 세개, 고고도일수록 잦아드는 방식
		}

	}

	
	if (_INPUT.GetButtonDown(KeyType::R))
	{
		launchGameMode->Reset();
		_ship->SetCenterPos(Vector2(0.f, -10.f));   // 발사대 위치는 여전히 LaunchWorld 책임
		_camera.SetPosition(_ship->GetCenterPos() + Vector2(0, -100.f));
		_camera.SetZoomImmediate(2.f);
		_cameraFollowStarted = false;
		_camera.SetFollowTarget(nullptr);

		_launchWidget->ResetWidget();
	}

	if (_INPUT.GetButtonDown(KeyType::P))
		WORLD.ChangeWorld("MainWorld");

	if (launchGameMode->GetLaunchState() == ELaunchState::Idle and _INPUT.GetButtonDown(KeyType::Enter))
	{
		_ship->SetAutoPilot(true);
		SOUND.Play(L"S_Countdown");
		launchGameMode->ChangeLaunchState(ELaunchState::Countdown);
	}

	

	_launchWidget->SetFuelRatio(_ship->GetFuel() / _ship->GetMaxFuel());
}
void LaunchWorld::Render(HDC hdc)
{
	float normalizedAltitude = Normalize(_altitude, 0.f, 1000.f);

	float density = expf(-_altitude / GAtmosphereScaleHeight);
	GetAtmosphereDensity(hdc, _altitude);

	_stars->Render(hdc, _camera, 1.f - density);

	Super::Render(hdc);

	//_lightingSystem->BeginRender(_camera);
	::BitBlt(_lightingSystem->GetBufferDC(), 0, 0, GWinSizeX, GWinSizeY, hdc, 0, 0, SRCCOPY);
	
	float sunT = 1.f - density;   // 0: 지상(하늘 밝아서 안 보임) ~ 1: 우주(하늘 검어서 또렷)
	Vector2 sunPos(GWinSizeX * 0.8f, GWinSizeY * 0.15f);   // 화면 우상단
	float sunRadius = 150.f + (150.f - 150.f) * sunT;
	BYTE sunIntensity = (BYTE)(190.f + (255.f - 190.f) * sunT);
	_lightingSystem->RenderSunGlow(sunPos, sunRadius, sunIntensity);

	BYTE cloudIntensity = (BYTE)(130.f * density);   // 대기 옅어지면(고도 오르면) 구름도 같이 옅어짐
	for (const CloudPuff& cloud : _clouds)
	{
		Vector2 screenPos = _camera.WorldToScreen(cloud.pos);
		float screenRadius = cloud.radius * _camera.GetZoom();
		_lightingSystem->RenderCloud(screenPos, screenRadius, cloudIntensity);
	}

	for (const SmokePuff& p : _launchSmoke)
	{
		Vector2 screenPos = _camera.WorldToScreen(p.pos);
		float lifeRatio = p.currentLife / p.maxLife;
		float screenRadius = p.radius * (1.6f - lifeRatio) * _camera.GetZoom();   // 태어날 때 작다가 점점 커짐
		BYTE alpha = (BYTE)(220.f * lifeRatio);   // 수명 다할수록 옅어짐
		_lightingSystem->RenderSmoke(screenPos, screenRadius, alpha);
	}


	if (_ship->GetIsThrusting())
	{
		Vector2 enginePos = _ship->GetCenterPos() - _ship->GetForwardDir() * 12.f;   // 선체 뒤쪽(엔진 위치)
		Vector2 dir = -_ship->GetForwardDir();

		_lightingSystem->SpawnEngineParticle(_camera.WorldToScreen(enginePos), dir);
		if (_altitude < 200.f)
		{
			_lightingSystem->SpawnEngineSmoke(_camera.WorldToScreen(enginePos), dir);
		}
		
	}
	_lightingSystem->UpdateAndRenderParticle(_lastDeltaTime);

	_lightingSystem->EndRender(hdc);




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

void LaunchWorld::GetAtmosphereDensity(HDC hdc, float altitude)
{
	//float topDensity = powf(density, 6.f);;
	//COLORREF topColor = ComputeRayleighSkyColor(topDensity);
	//COLORREF bottomColor = ComputeRayleighSkyColor(density);
	//
	//TRIVERTEX vert[2];
	//vert[0].x = 0;         vert[0].y = 0;
	//vert[0].Red = GetRValue(topColor) << 8;
	//vert[0].Green = GetGValue(topColor) << 8;
	//vert[0].Blue = GetBValue(topColor) << 8;
	//vert[0].Alpha = 0;
	//
	//vert[1].x = GWinSizeX;  vert[1].y = GWinSizeY;
	//vert[1].Red = GetRValue(bottomColor) << 8;
	//vert[1].Green = GetGValue(bottomColor) << 8;
	//vert[1].Blue = GetBValue(bottomColor) << 8;
	//vert[1].Alpha = 0;
	//
	//GRADIENT_RECT gRect = { 0, 1 };
	//::GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

	constexpr float kBandStartAltitude = 1500.f;   // 이 고도까지는 안 보임
	constexpr float kBandFullAltitude = 2000.f;   // 핸드오프 지점(2000)에서 완전히 자리잡음
	float t = clamp((altitude - kBandStartAltitude) / (kBandFullAltitude - kBandStartAltitude), 0.f, 1.f);

	constexpr float kBlackoutStartAltitude = 2000.f;   // 여기부터 다시 압축 시작
	constexpr float kBlackoutFullAltitude = 2450.f;   // 핸드오프 지점 — 거의 완전한 검정
	float blackout = clamp((altitude - kBlackoutStartAltitude) / (kBlackoutFullAltitude - kBlackoutStartAltitude), 0.f, 1.f);

	// 대기권 가장자리 색 구성 : 우주(검정) → 파랑 → 흰빛 하이라이트 → 주황 → 지상 하늘색
	COLORREF cSpace = RGB(5, 5, 15);
	COLORREF cBlue = RGB(30, 70, 150);
	COLORREF cBright = RGB(220, 235, 250);
	COLORREF cOrange = RGB(235, 130, 40);
	COLORREF cGround = ComputeRayleighSkyColor(1.f);

	// 4개 구간 경계. t = 0이면  전부 0으로 수렴 -> 화면 전체가 cGround
	// t가 커질수록(고도가 올라갈수록) 위에서부터 검정 -> 파랑 -> 흰색 -> 주항 차례로 두꺼워지면서 아래로 밀려난다.

	float blackEnd = 0.6f * t;
	float blueEnd = blackEnd + 0.1f * t;   // * → +
	float brightEnd = blueEnd + 0.05f * t;   // * → +
	float orangeEnd = brightEnd + 0.1f * t;   // 이어서 이 줄도 + 로

	blackEnd += (1.f - blackEnd) * blackout;
	blueEnd += (1.f - blueEnd) * blackout;
	brightEnd += (1.f - brightEnd) * blackout;
	orangeEnd += (1.f - orangeEnd) * blackout;

	float yRatio[5] = { 0.f, blackEnd, blueEnd, brightEnd, orangeEnd };
	COLORREF colors[5] = { cSpace, cBlue, cBright, cOrange, cGround };

	TRIVERTEX vert[8];
	GRADIENT_RECT gRect[4];

	for (int i = 0; i < 4; ++i)
	{
		TRIVERTEX& top = vert[i * 2];
		top.x = 0;
		top.y = (LONG)(GWinSizeY * clamp(yRatio[i], 0.f, 1.f));
		top.Red = GetRValue(colors[i]) << 8; top.Green = GetGValue(colors[i]) << 8; top.Blue = GetBValue(colors[i]) << 8; top.Alpha = 0;

		TRIVERTEX& bot = vert[i * 2 + 1];
		bot.x = GWinSizeX;
		bot.y = (LONG)(GWinSizeY * clamp(yRatio[i + 1], 0.f, 1.f));
		bot.Red = GetRValue(colors[i + 1]) << 8; bot.Green = GetGValue(colors[i + 1]) << 8; bot.Blue = GetBValue(colors[i + 1]) << 8; bot.Alpha = 0;

		gRect[i].UpperLeft = i * 2;
		gRect[i].LowerRight = i * 2 + 1;
	}

	::GradientFill(hdc, vert, 8, gRect, 4, GRADIENT_FILL_RECT_V);

	// 마지막 정지점(orangeEnd) 아래로 화면 끝까지는 지상색으로 채운다
	RECT groundRect{ 0, (LONG)(GWinSizeY * clamp(orangeEnd, 0.f, 1.f)), GWinSizeX, GWinSizeY };
	HBRUSH groundBrush = ::CreateSolidBrush(cGround);
	::FillRect(hdc, &groundRect, groundBrush);
	::DeleteObject(groundBrush);

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
