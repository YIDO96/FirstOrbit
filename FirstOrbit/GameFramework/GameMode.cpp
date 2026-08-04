#include "pch.h"
#include "GameMode.h"
#include "Core/GameInstance.h"

void GameMode::OnGUI()
{
	
	{
		ImVec2 wpos = ImGui::GetWindowPos();
		ImVec2 wsize = ImGui::GetWindowSize();
		GameInstance::GetInstance().RegisterUIBackgroundRect((int)wpos.x, (int)wpos.y, (int)wsize.x, (int)wsize.y);
	}
	ImGui::Text("GameState : %d", (int32)_state);
	
}
