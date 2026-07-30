#include "pch.h"
#include "EditorWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"

void EditorWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);


	if (_INPUT.GetButtonDown(KeyType::F1))
		WORLD.ChangeWorld("TitleWorld");
}

void EditorWorld::Render(HDC hdc)
{
}

void EditorWorld::OnSceneGUI()
{
	ImGui::Text("F1 : 타이틀로 돌아가기");
}
