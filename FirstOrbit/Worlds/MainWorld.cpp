#include "pch.h"
#include "MainWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/ResourceManager.h"
#include "Core/GameInstance.h"

#include "GameFramework/Texture.h"


void MainWorld::Enter()
{
	Super::Enter();

	LoadTexture();

	_camera.Init(GWinSizeX, GWinSizeY);
}

void MainWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	
	// 아직 게임플레이 콘텐츠가 없어서, G 키로 게임오버 전환을 임시로 시연한다.
	if (_INPUT.GetButtonDown(KeyType::G))
		WORLD.ChangeWorld("GameOverWorld");


	_camera.Update(deltaTime);
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

	Super::Render(hdc);

	


	// 3-5 버그 재현용 — 월드 격자점을 WorldToScreen으로 변환해서 화면에 찍어본다
	HBRUSH brush = ::CreateSolidBrush(RGB(255, 0, 0));
	HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, brush);

	for (int x = -200; x <= 200; x += 50)
	{
		for (int y = -200; y <= 200; y += 50)
		{
			Vector2 worldPos = Vector2((float)x, (float)y);
			Vector2 screenPos = _camera.WorldToScreen(worldPos);

			::Ellipse(hdc,
				(int)screenPos.x - 3, (int)screenPos.y - 3,
				(int)screenPos.x + 3, (int)screenPos.y + 3);
		}
	}

	::SelectObject(hdc, oldBrush);
	::DeleteObject(brush);
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

void MainWorld::OnSceneGUI()
{
	ImGui::Text("Gameplay (아직 액터 없음)");
	ImGui::Text("G : 게임오버 전환 (데모)");

	_camera.OnSceneGUI();

	ImGui::Text("WheelDelta : %.2f", _INPUT.GetWheelDelta());
	//ImGui::Text("BG Pos X : %.2f, Y : ", _bg.);
}
