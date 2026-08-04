#include "pch.h"
#include "GameMode.h"
#include "Core/GameInstance.h"

void GameMode::OnGUI()
{
	switch (_state)
	{
	case EGameState::Playing:
		ImGui::Text("GameState : Playing");
		break;
	case EGameState::GameOver:
		ImGui::Text("GameState : GameOver");
		break;
	default:
		break;
	}
}
