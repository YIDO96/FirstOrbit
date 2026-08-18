#include "pch.h"
#include "MainWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/ResourceManager.h"
#include "Core/GameInstance.h"
#include "Core/UIManager.h"
#include "Core/SoundManager.h"

#include "GameFramework/Texture.h"
#include "GameFramework/Components/UPhysicsComponent.h"
#include "Actor/APlanet.h"
#include "Actor/ASpaceship.h"

#include "GameSystem/SpaceLightingSystem.h"
#include "GameSystem/InterplanetaryTransfer.h"
#include "GameMode/OrbitalGameMode.h"

#include "Actor/StarField.h"
#include "Actor/ABlackHole.h"

#include "Widget/Widget_Main.h"

MainWorld::~MainWorld()
{
	//delete _starField;
}

void MainWorld::Enter()
{
	Super::Enter();

	LoadTexture();
	
	_camera.SetIsControll(true);
	
	SOUND.Play(L"S_Space", true);
	SOUND.SetVolume(L"S_Space", 0.1f);

	InitPlanet();

	_ship = SpawnActor<ASpaceship>();
	//_ship->SetCenterPos(_homePlanet->GetCenterPos() + Vector2(1300.f, 0.f));
	_ship->SetTargetPlanet(_homePlanet);

	LaunchHandoff handoff;
	if (GAME.ConsumeLaunchHandoff(handoff))
	{
		Vector2 launchOrigin = _homePlanet->GetCenterPos() + Vector2(0.f, -_homePlanet->GetBodyRadius());
		//_ship->SetCenterPos(launchOrigin + handoff.position);
		_ship->SetCenterPos(_homePlanet->GetCenterPos() + handoff.position);
		_ship->SetRotation(handoff.degree);
		_ship->GetComponent<UPhysicsComponent>()->SetVelocity(handoff.velocity + _homePlanet->GetVelocity());


		if (handoff.autoOrbit)
		{
			UPhysicsComponent* physics = _ship->GetComponent<UPhysicsComponent>();
			Vector2 center = _homePlanet->GetCenterPos();
			float r = 1200.f;   // 고도 300 — KEY_0이랑 동일한 값
			float speed = sqrtf(_homePlanet->GetMu() / r);

			Vector2 dir(0.f, -1.f);
			Vector2 perp(-dir.y, dir.x);

			_ship->SetCenterPos(center + dir * r);
			physics->SetVelocity(perp * speed + _homePlanet->GetVelocity());
		}
	}
	else
	{
		_ship->SetCenterPos(_homePlanet->GetCenterPos() + Vector2(0.f, -1300.f));
		_ship->GetComponent<UPhysicsComponent>()->SetVelocity(_homePlanet->GetVelocity());
	}

	_ship->GetComponent<UPhysicsComponent>()->SetPaused(false);
	GetGameMode<OrbitalGameMode>()->SetShip(_ship);
	_homePlanet = _ship->GetTargetPlanet();
	_camera.SetFollowTarget(_ship);					// 관찰하기 쉽게 카메라 붙여두기
	_camera.SetZoomImmediate(0.3f);

	//_starField = new StarField();
	//_starField->Init(20000, 500000.f);
	_lightingSystem = new SpaceLightingSystem();
	HDC hdc = ::GetDC(GAME.GetHwnd());
	_lightingSystem->Initialize(hdc, GWinSizeX, GWinSizeY);
	::ReleaseDC(GAME.GetHwnd(), hdc);
	_lightingSystem->GenerateStarfield(600000, 5000000.f);
	_lightingSystem->AddBlackHoleFallingStars(_blackHole->GetCenterPos(), 20000, 5500.f, _blackHole->GetEventHorizonRadius());


	_widget = UI.CreateWidget<Widget_Main>();
	_widget->SetOwnerWorld(this);
	_widget->BindShip(_ship);

	// 우주선 + 태양계(태양 포함) + 블랙홀, 총 11개 액터 버튼을 한 번만 만든다.
	vector<AActor*> actorButtonList;
	actorButtonList.push_back(_ship);
	for (APlanet* planet : _planets) actorButtonList.push_back(planet);
	actorButtonList.push_back(_blackHole);

	_widget->ShowActorButtons(actorButtonList);
	_widget->UpdateActorButtonStates(false, {});   // 시작 상태: 우주선 안 픽힘 → 전부 활성

	_widget->SetOnActorSelected([this](AActor* actor)
		{
			SelectActor(actor);
		});
}

void MainWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);
	_lastDeltaTime = deltaTime;

	if (_transferTarget and not _transferCaptured)
	{
		float soi = _transferTarget->GetSOIRadius(_sun->GetMu());
		Vector2 toShip = _ship->GetCenterPos() - _transferTarget->GetCenterPos();

		if (toShip.LengthSquared() <= soi * soi)
		{
			UPhysicsComponent* physics = _ship->GetComponent<UPhysicsComponent>();
			float r = max(toShip.Length(), 1.f);
			Vector2 dir = toShip / r;
			Vector2 perp(-dir.y, dir.x);
			//float speed = sqrtf(_transferTarget->GetMu() / r);

			Vector2 relVel = physics->GetVelocity() - _transferTarget->GetVelocity();
			float cross = dir.x * relVel.y - dir.y * relVel.x;
			if (cross < 0.f) perp = -perp;

			float targetR = _transferTarget->GetBodyRadius() + 300.f;
			float speed = sqrtf(_transferTarget->GetMu() / targetR);

			_ship->SetCenterPos(_transferTarget->GetCenterPos() + dir * targetR);
			physics->SetVelocity(perp * speed + _transferTarget->GetVelocity());
			_ship->SetForceHeliocentric(false, 0.f);
			_transferCaptured = true;
		}
	}
	

	
	
	// 아직 게임플레이 콘텐츠가 없어서, G 키로 게임오버 전환을 임시로 시연한다.
	if (_INPUT.GetButtonDown(KeyType::P))
		WORLD.ChangeWorld("GameOverWorld");


	if (_INPUT.GetButtonDown(KeyType::LeftMouse))
	{
		ImGuiIO& io = ImGui::GetIO();
		Vector2 mousePos = _INPUT.GetMousePos();
		bool overOwnUI = _widget and _widget->IsMouseOverUI(mousePos);
		if (_INPUT.IsMouseInsideWindow(GAME.GetGameViewportRect()) and not io.WantCaptureMouse and not overOwnUI)
		{
			Vector2 worldPos = _camera.WorldToMousePos(mousePos);
			if (AActor* picked = PickActor(worldPos))
				SelectActor(picked);
			else
				DeselectActor();   // 빈 공간 클릭 → 픽/선택 해제
		}
	}

	if (_INPUT.GetButtonDown(KeyType::KEY_0))
	{
		UPhysicsComponent* physics = _ship->GetComponent<UPhysicsComponent>();
		Vector2 center = _homePlanet->GetCenterPos();
		float r = 1300.f;	// 고도 300 (지구 반지금 1000)
		float speed = sqrtf(_homePlanet->GetMu() / r);

		Vector2 dir(0.f, -1.f);						// 행성 -> 우주선 방향
		Vector2 perp(-dir.y, dir.x);				// 반지름에 수직
		
		_ship->SetCenterPos(center + dir * r);
		physics->SetVelocity(perp * speed + _homePlanet->GetVelocity());
		GetGameMode<OrbitalGameMode>()->ResetJudgment();
	}

	// 블랙홀 근처로 이동시키기
	if (_INPUT.GetButtonDown(KeyType::KEY_1))
	{
		UPhysicsComponent* physics = _ship->GetComponent<UPhysicsComponent>();
		Vector2 center = _blackHole->GetCenterPos();
		float r = _blackHole->GetEventHorizonRadius() * 3.f;   // 이벤트 호라이즌 바로 바깥 — 렌즈효과가 잘 보이는 거리
		float speed = sqrtf(_blackHole->GetMu() / r);

		Vector2 dir(0.f, -1.f);
		Vector2 perp(-dir.y, dir.x);

		_ship->SetCenterPos(center + dir * r);
		physics->SetVelocity(perp * speed);   // 블랙홀은 고정 위치라 자체 속도를 더할 필요 없음
		GetGameMode<OrbitalGameMode>()->ResetJudgment();

		_camera.SetPosition(_ship->GetCenterPos());   // 추가 — 보간 없이 즉시 스냅
		_camera.SetZoomImmediate(0.02f);   // 추가 — 이 스케일(15000 유닛)에 맞는 줌
	}

	{
		_widget->SetFuelRatio(_ship->GetFuel() / _ship->GetMaxFuel());
		_widget->SetHeading(DegreeToRadian(_ship->GetDegree()));
	}

	_lightingSystem->UpdateStars(deltaTime);
}

void MainWorld::Render(HDC hdc)
{
	//_starField->Render(hdc, _camera);

	_lightingSystem->BeginRender(_camera);

	// 태양 빛
	_lightingSystem->RenderSunGlow(_camera.WorldToScreen(_sun->GetCenterPos()),
		_camera.WorldToScreenScale(_sun->GetBodyRadius() * 15.f), 140);

	// 행성 그림자
	for (auto actor : _actors)
	{
		if (actor->GetType() != EActorType::Planet) continue;

		if (actor->GetName() == "Sun") continue;
		APlanet* planet = static_cast<APlanet*>(actor);

		_lightingSystem->ApplyPlanetShadow(
			_camera.WorldToScreen(_sun->GetCenterPos()),
			_camera.WorldToScreen(planet->GetCenterPos()),
			_camera.WorldToScreenScale(planet->GetBodyRadius()));
	}
	
	float bhScreenRadius = _camera.WorldToScreenScale(_blackHole->GetEventHorizonRadius());
	bhScreenRadius = clamp(bhScreenRadius, 15.f, 80.f);

	// 블랙홀
	_lightingSystem->ApplyBlackHoleLensing(
		_camera.WorldToScreen(_blackHole->GetCenterPos()),
		bhScreenRadius,100.f);

	_lightingSystem->EndRender(hdc);

	if (_ship->GetIsThrusting())
	{
		Vector2 enginePos = _ship->GetCenterPos() - _ship->GetForwardDir() * 12.f;   // 선체 뒤쪽(엔진 위치)
		_lightingSystem->SpawnEngineParticle(_camera.WorldToScreen(enginePos), -_ship->GetForwardDir());
	}
	_lightingSystem->UpdateAndRenderParticle(_lastDeltaTime);

	// 행성 궤도 모양 (은은한 점섬)
	for (auto planet : _planets)
	{
		if (planet == _sun) continue;

		vector<Vector2> shape = planet->GetOrbitShape(150);
		for (const Vector2& p : shape)
		{
			Vector2 s = _camera.WorldToScreen(p);
			::SetPixel(hdc, (int)s.x, (int)s.y, RGB(255, 255, 0));
		}
	}

	for (Vector2 pos : _transferPath)
	{
		Vector2 s = _camera.WorldToScreen(pos);
		::SetPixel(hdc, (int)s.x, (int)s.y, RGB(255, 255, 255));
	}



	Super::Render(hdc);

	vector<Vector2> predicted = GetGameMode<OrbitalGameMode>()->PredictPath(200, 0.5f);
	for (const Vector2& p : predicted)
	{
		Vector2 s = _camera.WorldToScreen(p);
		::SetPixel(hdc, (int)s.x, (int)s.y, RGB(0, 255, 255));
	}
}

void MainWorld::LoadTexture() 
{

}

void MainWorld::SetCameraOnGUIDoubleClickedEvent(AActor* target)
{
	Super::SetCameraOnGUIDoubleClickedEvent(target);

	SelectActor(target);
}

vector<APlanet*> MainWorld::GetReachablePlanets() const
{
	// 최단(직행) 전이가 아니라 크게 삥 도는 전이만 나오는 행성은 버튼에서 제외한다.
	// 기준: 예상 궤도의 태양 기준 최대거리가 출발/도착 거리 중 큰 쪽의 1.5배를 넘으면 "삥 도는 것"으로 판단.
	//  - 이상적인 호만전이는 정확히 1.0배(원일점 = max(r1,r2))라 1.5배면 위상이 살짝 안 맞는 정도는 봐주고,
	//    위상이 많이 어긋나 궤도가 크게 부푸는 경우만 걸러낸다.
	constexpr float kMaxApoapsisRatio = 1.5f;

	vector<APlanet*> result;
	UPhysicsComponent* physics = _ship->GetComponent<UPhysicsComponent>();
	float r1 = _ship->GetCenterPos().Length();

	for (APlanet* planet : _planets)
	{
		TransferPlan plan = FindBestTransfer(_ship->GetCenterPos(), physics->GetVelocity(), planet, _sun->GetMu());
		if (not plan.valid) continue;

		vector<Vector2> path = PredictTransferPath(_ship->GetCenterPos(), plan.v1, _sun->GetCenterPos(), _sun->GetMu(), plan.tof, 60);
		float maxDist = 0.f;
		for (const Vector2& p : path)
			maxDist = max(maxDist, (p - _sun->GetCenterPos()).Length());

		float r2 = planet->GetCenterPos().Length();

		if (maxDist <= kMaxApoapsisRatio * max(r1, r2))
			result.push_back(planet);
	}

	return result;
}

void MainWorld::SelectActor(AActor* actor)
{
	if (not actor) return;

	if (actor == _ship)
	{
		// 우주선 버튼(혹은 뷰포트에서 우주선 클릭) → "목적지 선택 모드"로 진입.
		_selected = _ship;
		_camera.SetFollowTarget(_ship);
		_widget->UpdateActorButtonStates(true, GetReachablePlanets());
		SOUND.Play(L"S_Spaceship", true);
		return;
	}
	else
	{
		SOUND.Stop(L"S_Spaceship");
	}

	if (_selected == _ship)
	{
		// 이미 우주선이 픽힌 상태에서 행성을 고르면 전이 시작. 행성이 아니면(블랙홀 등) 그냥 포커스.
		if (APlanet* planet = dynamic_cast<APlanet*>(actor))
		{
			StartTransfer(planet);
			return;
		}
	}

	// 그 외엔 그냥 포커스만 (카메라 따라가기 + 디버그 패널용 선택 상태).
	_selected = actor;
	_camera.SetFollowTarget(actor);
	_widget->UpdateActorButtonStates(false, {});

	if (_selected == _blackHole)
	{
		SOUND.Play(L"S_BlackHole", true);
	}
	else
	{
		SOUND.Stop(L"S_BlackHole");
	}
}

void MainWorld::StartTransfer(APlanet* target)
{
	UPhysicsComponent* physics = _ship->GetComponent<UPhysicsComponent>();
	TransferPlan plan = FindBestTransfer(_ship->GetCenterPos(), physics->GetVelocity(), target, _sun->GetMu());

	if (plan.valid)
	{
		_transferPath = PredictTransferPath(_ship->GetCenterPos(), plan.v1, _sun->GetCenterPos(), _sun->GetMu(), plan.tof, 200);

		float maxDist = 0.f;
		for (const Vector2& p : _transferPath)
			maxDist = max(maxDist, (p - _sun->GetCenterPos()).Length());
		_transferApoapsis = maxDist;

		_ship->SetTargetPlanet(_sun);
		_ship->SetForceHeliocentric(true, plan.tof);
		physics->SetVelocity(plan.v1);

		_transferTarget = target;
		_transferAngleDeg = plan.transferAngle * (180.f / 3.14159265f);
		_transferCaptured = false;
	}

	// 전이를 걸었으니(성공/실패 여부와 무관하게) 목적지 선택 모드는 종료 — 버튼은 다시 전부 활성.
	_selected = nullptr;
	_widget->UpdateActorButtonStates(false, {});
}

void MainWorld::DeselectActor()
{
	if (_selected == _blackHole)
	{
		SOUND.Stop(L"S_BlackHole");
	}
	if (_selected == _ship)
	{
		SOUND.Stop(L"S_Spaceship");
	}

	_selected = nullptr;
	_camera.SetFollowTarget(nullptr);
	_widget->UpdateActorButtonStates(false, {});
}

void MainWorld::InitPlanet()
{
	// 태양계 행성들을 생성하고 초기화
	{
		APlanet* sun = SpawnActor<APlanet>();
		sun->Setup("Sun", Vector2(0, 0), 0.f, 0.f, 2.88e9f, 12000.f);
		sun->SetTexture(RESOURCE.GetTexture(L"Sun"));
		_sun = sun;
		_planets.push_back(_sun);

		{ // 수성
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* mercury = SpawnActor<APlanet>();//0.032f
			mercury->Setup("Mercury", sun->GetCenterPos(), 20000.f, 0.032f, 3.2e6f, 400.f, randomAngle, 0.206f, periAngle);
			mercury->SetTexture(RESOURCE.GetTexture(L"Mercury"));

			_planets.push_back(mercury);
		}

		{ // 금성
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* venus = SpawnActor<APlanet>();
			venus->Setup("Venus", sun->GetCenterPos(), 40000.f, 0.024f, 1.805e7f, 950.f, randomAngle, 0.007f, periAngle);
			venus->SetTexture(RESOURCE.GetTexture(L"Venus"));

			_planets.push_back(venus);
		}

		{ // 지구
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* earth = SpawnActor<APlanet>();
			earth->Setup("Earth", sun->GetCenterPos(), 60000.f, 0.02f, 2.0e7f, 1000.f, randomAngle, 0.017f, periAngle);
			earth->SetTexture(RESOURCE.GetTexture(L"Earth"));
			_homePlanet = earth;
			_camera.SetPosition(earth->GetCenterPos());

			_planets.push_back(earth);
		}


		{ // 화성
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* mars = SpawnActor<APlanet>();
			mars->Setup("Mars", sun->GetCenterPos(), 80000.f, 0.016f, 6.05e6f, 550.f, randomAngle, 0.093f, periAngle);
			mars->SetTexture(RESOURCE.GetTexture(L"Mars"));

			_planets.push_back(mars);
		}

		{ // 목성
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* jupiter = SpawnActor<APlanet>();
			jupiter->Setup("Jupiter", sun->GetCenterPos(), 120000.f, 0.009f, 3.2e8f, 4000.f, randomAngle, 0.048f, periAngle);
			jupiter->SetTexture(RESOURCE.GetTexture(L"Jupiter"));

			_planets.push_back(jupiter);
		}

		{ // 토성
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* saturn = SpawnActor<APlanet>();
			saturn->Setup("Saturn", sun->GetCenterPos(), 160000.f, 0.007f, 2.048e8f, 3200.f, randomAngle, 0.056f, periAngle);
			saturn->SetTexture(RESOURCE.GetTexture(L"Jupiter"));

			_planets.push_back(saturn);
		}

		{ // 천왕성
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* uranus = SpawnActor<APlanet>();
			uranus->Setup("Uranus", sun->GetCenterPos(), 200000.f, 0.005f, 9.68e7f, 2200.f, randomAngle, 0.046f, periAngle);
			uranus->SetTexture(RESOURCE.GetTexture(L"Uranus"));

			_planets.push_back(uranus);
		}

		{ // 해왕성
			float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
			float periAngle = ((float)rand() / RAND_MAX) * 6.283185f;
			APlanet* neptune = SpawnActor<APlanet>();
			neptune->Setup("Neptune", sun->GetCenterPos(), 240000.f, 0.004f, 8.0e7f, 2000.f, randomAngle, 0.01f, periAngle);
			neptune->SetTexture(RESOURCE.GetTexture(L"Neptune"));

			_planets.push_back(neptune);
		}
	}

	{
		_blackHole = SpawnActor<ABlackHole>();
		_blackHole->Setup(Vector2(500000.f, 300000.f), 1.0e10f, 5000.f);
	}
}

void MainWorld::UpdateUIData()
{
	OrbitalGameMode* gm = GetGameMode<OrbitalGameMode>();
	UPhysicsComponent* physics = _ship->GetComponent<UPhysicsComponent>();
	APlanet* target = _ship->GetTargetPlanet();
	APlanet* home = gm->GetHomePlanet();

	const wchar_t* stateNames[] = { L"비행 중", L"궤도 안착", L"실패" };

	wchar_t buf[128];
	swprintf_s(buf, L"속도: %.1f", physics->GetVelocity().Length());
	_widget->SetLine(0, buf);

	swprintf_s(buf, L"근지점: %.1f", gm->GetPerigee());
	_widget->SetLine(1, buf);

	swprintf_s(buf, L"원지점: %.1f", gm->GetApogee());
	_widget->SetLine(2, buf);

	_widget->SetLine(3, stateNames[(int32)gm->GetOrbitState()]);

	swprintf_s(buf, L"중력원: %s", target ? CharToWStringStandard(target->GetName().c_str()).c_str() : L"없음");
	_widget->SetLine(4, buf);

	_widget->SetFuelRatio(_ship->GetFuel() / _ship->GetMaxFuel());
}

void MainWorld::OnSceneGUI()
{
	ImGui::Text("Gameplay (아직 액터 없음)");
	ImGui::Text("P : 게임오버 전환 (데모)");

	if (_selected)
	{
		if (APlanet* p = dynamic_cast<APlanet*>(_selected))
			ImGui::Text("선택된 행성: %s", p->GetName().c_str());
	}
	if (_transferTarget)
	{
		float r1 = _ship->GetCenterPos().Length();          // 출발 시 태양으로부터 거리(근사)
		float r2 = _transferTarget->GetCenterPos().Length();
		ImGui::Text("전이 최대거리: %.0f (출발 %.0f / 목표 %.0f)", _transferApoapsis, r1, r2);
	}
	if (APlanet* mercury = _planets.size() > 1 ? _planets[1] : nullptr)   // 인덱스는 실제 순서에 맞게
	{
		float dist = (mercury->GetCenterPos() - _sun->GetCenterPos()).Length();
		ImGui::Text("수성 현재 거리: %.1f (근일점 예상 15880 / 원일점 예상 24120)", dist);
	}

	if (APlanet* p = dynamic_cast<APlanet*>(_selected))
	{
		float err = ComputePhaseAngleError(_ship->GetCenterPos(), p, _sun->GetMu());
		ImGui::Text("위상 오차: %.1f도 (0에 가까울수록 직행)", err);
	}

	//if (_blackHole)
	//{
	//	Vector2 bhPos = _camera.WorldToScreen(_blackHole->GetCenterPos());
	//	float bhRadius = _camera.WorldToScreenScale(_blackHole->GetEventHorizonRadius());
	//	float effectRadius = bhRadius * 3.5f;
	//	ImGui::Text("bhPos: %.1f, %.1f", bhPos.x, bhPos.y);
	//	ImGui::Text("bhRadius: %.2f", bhRadius);
	//	ImGui::Text("effectRadius: %.2f", effectRadius);
	//	ImGui::Text("Glow destSize: %.2f", (effectRadius * 0.8f) * 2.f);
	//	ImGui::Text("Zoom: %.5f", _camera.GetZoom());
	//	ImGui::Text("AlphaBlend 성공?: %s", _lightingSystem->GetDebugLastGlowOk() ? "true" : "false");
	//}

}

GameMode* MainWorld::CreateGameMode()
{
	return new OrbitalGameMode();
}
