#include "pch.h"
#include "OrbitalGameMode.h"

void OrbitalGameMode::Update(float deltaTime)
{
	Super::Update(deltaTime);
}

void OrbitalGameMode::OnGUI()
{
	if (ImGui::TreeNode("GameMode"))
	{
		ImGui::Text("GameMode : OrbitalGameMode");
		Super::OnGUI();

		ImGui::TreePop();
	}
}
