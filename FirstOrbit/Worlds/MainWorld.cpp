#include "pch.h"
#include "MainWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/ResourceManager.h"
#include "Core/GameInstance.h"

#include "GameFramework/Texture.h"
#include "Actor/APlanet.h"

#include "StarField.h"

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

	_starField = new StarField();
	_starField->Init(20000, 500000.f);
}

void MainWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	
	// 아직 게임플레이 콘텐츠가 없어서, G 키로 게임오버 전환을 임시로 시연한다.
	if (_INPUT.GetButtonDown(KeyType::G))
		WORLD.ChangeWorld("GameOverWorld");


	if (_INPUT.GetButtonDown(KeyType::LeftMouse))
	{
		Vector2 worldPos = _camera.WorldToMousePos(_INPUT.GetMousePos());
		_selected = PickActor(worldPos);
		_camera.SetFollowTarget(_selected);
	}
}

void MainWorld::Render(HDC hdc)
{
	//for (int i = 0; i < 5; i++)
	//{
	//	for (int j = 0; j < 5; j++)
	//	{
	//		if (_bgs[i * 5 + j])
	//		{
	//			Vector2 imageSize = _bgs[i * 5 + j]->GetTextureSize();
	//			Vector2 worldPos = Vector2(-imageSize.x * 2.5f + (imageSize.x-10) * j, -imageSize.y * 2.5f + imageSize.y * i);
	//
	//			Vector2 screenCenter = _camera.WorldToScreen(worldPos);
	//			Vector2 screenSize = Vector2(
	//				_camera.WorldToScreenScale(imageSize.x),
	//				_camera.WorldToScreenScale(imageSize.y)
	//			);
	//
	//			Vector2 renderPos = screenCenter - screenSize / 2.f;
	//
	//			//_bg->Render(hdc, renderPos, Vector2(), screenSize);
	//			_bgs[i * 5 + j]->Render(hdc, renderPos, Vector2(), Vector2(), screenSize);
	//		}
	//	}
	//}

	//if (_bg)
	//{
	//	Vector2 worldPos = Vector2(0, 0);
	//	Vector2 imageSize = _bg->GetTextureSize();
	//
	//	Vector2 screenCenter = _camera.WorldToScreen(worldPos);
	//	Vector2 screenSize = Vector2(
	//		_camera.WorldToScreenScale(imageSize.x),
	//		_camera.WorldToScreenScale(imageSize.y)
	//	);
	//
	//	Vector2 renderPos = screenCenter - screenSize / 2.f;
	//
	//	//_bg->Render(hdc, renderPos, Vector2(), screenSize);
	//	_bg->Render(hdc, renderPos, Vector2(), Vector2(), screenSize);
	//}
	_starField->Render(hdc, _camera);

	Super::Render(hdc);

	// 3-5 버그 재현용 — 월드 격자점을 WorldToScreen으로 변환해서 화면에 찍어본다
	//HBRUSH brush = ::CreateSolidBrush(RGB(255, 0, 0));
	//HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);
	//
	//for (int x = -200; x <= 200; x += 50)
	//{
	//	for (int y = -200; y <= 200; y += 50)
	//	{
	//		Vector2 worldPos = Vector2((float)x, (float)y);
	//		Vector2 screenPos = _camera.WorldToScreen(worldPos);
	//
	//		::Ellipse(hdc,
	//			(int)screenPos.x - 3, (int)screenPos.y - 3,
	//			(int)screenPos.x + 3, (int)screenPos.y + 3);
	//	}
	//}
	//::SelectObject(hdc, oldBrush);
	//::DeleteObject(brush);
}

void MainWorld::LoadTexture() 
{
	//_bg = RESOURCE.GetTexture(L"BG");
	
	//for (int i = 0; i < 5; i++)
	//{
	//	for (int j = 0; j < 5; j++)
	//	{
	//		_bgs[i * 5 + j] = RESOURCE.GetTexture(L"BG");
	//	}
	//}
}

void MainWorld::InitPlanet()
{
	APlanet* sun = SpawnActor<APlanet>();
	sun->Setup("Sun", Vector2(0, 0), 0.f,  0.f, 2.88e9f, 12000.f);
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

void MainWorld::OnSceneGUI()
{
	ImGui::Text("Gameplay (아직 액터 없음)");
	ImGui::Text("G : 게임오버 전환 (데모)");

	//_camera.OnSceneGUI();

	ImGui::Text("WheelDelta : %.2f", _INPUT.GetWheelDelta());
	//ImGui::Text("BG Pos X : %.2f, Y : ", _bg.);

	if (_selected)
	{
		if (APlanet* p = dynamic_cast<APlanet*>(_selected))
			ImGui::Text("선택된 행성: %s", p->GetName().c_str());
	}
}
