#include "pch.h"
#include "GameMode.h"
#include "Core/GameInstance.h"

void GameMode::OnGUI()
{
	const char* stateNames[] = { "Playing", "GameOver" };
	ImGui::Text("GameModeState : %s", stateNames[(int32)_state]);
}
