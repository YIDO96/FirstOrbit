#include "pch.h"
#include "TitleWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"

void TitleWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (_INPUT.GetButtonDown(KeyType::SpaceBar))
		WORLD.ChangeWorld("MainWorld");

	if (_INPUT.GetButtonDown(KeyType::F1))
		WORLD.ChangeWorld("EditorWorld");
}

void TitleWorld::OnSceneGUI()
{
	ImGui::Text("F1 : EditorWorld로 진입");
	ImGui::Text("SpaceBar : 게임 시작");
}
