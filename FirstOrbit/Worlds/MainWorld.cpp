#include "pch.h"
#include "MainWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/ResourceManager.h"
#include "Core/GameInstance.h"

#include "GameFramework/Texture.h"
#include "GameFramework/Components/UPhysicsComponent.h"
#include "Actor/APlanet.h"
#include "Actor/ASpaceship.h"

#include "GameMode/OrbitalGameMode.h"

#include "Actor/StarField.h"

MainWorld::~MainWorld()
{
	delete _starField;
}

void MainWorld::Enter()
{
	Super::Enter();

	LoadTexture();
	
	_camera.SetIsControll(true);
	_camera.SetZoomImmediate(0.01f);


	InitPlanet();

	_ship = SpawnActor<ASpaceship>();
	//_ship->SetCenterPos(_homePlanet->GetCenterPos() + Vector2(1300.f, 0.f));
	_ship->SetTargetPlanet(_homePlanet);

	LaunchHandoff handoff;
	if (GAME.ConsumeLaunchHandoff(handoff))
	{
		Vector2 launchOrigin = _homePlanet->GetCenterPos() + Vector2(0.f, -_homePlanet->GetBodyRadius());
		_ship->SetCenterPos(launchOrigin + handoff.position);
		//_ship->SetCenterPos(_homePlanet->GetCenterPos() + handoff.position);
		_ship->SetRotation(handoff.degree);
		_ship->GetComponent<UPhysicsComponent>()->SetVelocity(handoff.velocity);
	}
	else
	{
		_ship->SetCenterPos(_homePlanet->GetCenterPos() + Vector2(0.f, -1300.f));
	}

	_ship->GetComponent<UPhysicsComponent>()->SetPaused(false);
	GetGameMode<OrbitalGameMode>()->SetShip(_ship);
	_homePlanet = _ship->GetTargetPlanet();
	_camera.SetFollowTarget(_ship);					// 관찰하기 쉽게 카메라 붙여두기


	_starField = new StarField();
	_starField->Init(20000, 500000.f);

}

void MainWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	
	// 아직 게임플레이 콘텐츠가 없어서, G 키로 게임오버 전환을 임시로 시연한다.
	if (_INPUT.GetButtonDown(KeyType::P))
		WORLD.ChangeWorld("GameOverWorld");


	if (_INPUT.GetButtonDown(KeyType::LeftMouse))
	{
		Vector2 worldPos = _camera.WorldToMousePos(_INPUT.GetMousePos());
		_selected = PickActor(worldPos);
		_camera.SetFollowTarget(_selected);
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
		physics->SetVelocity(perp * speed);
	}
}

void MainWorld::Render(HDC hdc)
{
	_starField->Render(hdc, _camera);

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

void MainWorld::InitPlanet()
{
	// 태양계 행성들을 생성하고 초기화
	{
		APlanet* sun = SpawnActor<APlanet>();
		sun->Setup("Sun", Vector2(0, 0), 0.f, 0.f, 2.88e9f, 12000.f);
		sun->SetTexture(RESOURCE.GetTexture(L"Sun"));

		float randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* mercury = SpawnActor<APlanet>();//0.032f
		mercury->Setup("Mercury", sun->GetCenterPos(), 20000.f, 0.032f, 3.2e6f, 400.f, randomAngle);
		mercury->SetTexture(RESOURCE.GetTexture(L"Earth"));

		randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* venus = SpawnActor<APlanet>();
		venus->Setup("Venus", sun->GetCenterPos(), 40000.f, 0.024f, 1.805e7f, 950.f, randomAngle);
		venus->SetTexture(RESOURCE.GetTexture(L"Earth"));

		randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* earth = SpawnActor<APlanet>();
		earth->Setup("Earth", sun->GetCenterPos(), 60000.f, 0.02f, /*mu*/ 	2.0e7f, /*bodyRadius*/ 1000.f, randomAngle);
		earth->SetTexture(RESOURCE.GetTexture(L"Earth"));
		_homePlanet = earth;
		_camera.SetPosition(earth->GetCenterPos());


		randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* mars = SpawnActor<APlanet>();
		mars->Setup("Mars", sun->GetCenterPos(), 80000.f, 0.016f, 6.05e6f, 550.f, randomAngle);
		mars->SetTexture(RESOURCE.GetTexture(L"Earth"));

		randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* jupiter = SpawnActor<APlanet>();
		jupiter->Setup("Jupiter", sun->GetCenterPos(), 120000.f, 0.009f, 3.2e8f, 4000.f, randomAngle);
		jupiter->SetTexture(RESOURCE.GetTexture(L"Earth"));

		randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* saturn = SpawnActor<APlanet>();
		saturn->Setup("Saturn", sun->GetCenterPos(), 160000.f, 0.007f, 2.048e8f, 3200.f, randomAngle);
		saturn->SetTexture(RESOURCE.GetTexture(L"Earth"));

		randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* uranus = SpawnActor<APlanet>();
		uranus->Setup("Uranus", sun->GetCenterPos(), 200000.f, 0.005f, 9.68e7f, 2200.f, randomAngle);
		uranus->SetTexture(RESOURCE.GetTexture(L"Earth"));

		randomAngle = ((float)rand() / RAND_MAX) * 6.283185f; // 0 ~ 2*PI 랜덤
		APlanet* neptune = SpawnActor<APlanet>();
		neptune->Setup("Neptune", sun->GetCenterPos(), 240000.f, 0.004f, 8.0e7f, 2000.f, randomAngle);
		neptune->SetTexture(RESOURCE.GetTexture(L"Earth"));
	}
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

	//if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
	//{
	//
	//}
}

GameMode* MainWorld::CreateGameMode()
{
	return new OrbitalGameMode();
}
