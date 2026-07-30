#include "pch.h"
#include "GameOverWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"

void GameOverWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (InputManager::GetInstance().GetButtonDown(KeyType::SpaceBar))
		WORLD.ChangeWorld("TitleWorld");
}

void GameOverWorld::OnSceneGUI()
{
	ImGui::Text("Game Over");
	ImGui::Text("SpaceBar : 타이틀로");
}
