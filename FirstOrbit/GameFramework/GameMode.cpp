#include "pch.h"
#include "GameMode.h"
#include "Core/GameInstance.h"

void GameMode::OnGUI()
{
	// NoBackground + RegisterUIBackgroundRect: GameInstance.h 주석 참고
	// (ImGui 소프트웨어 래스터라이저 대신 GDI로 배경을 채워 성능을 아낀다)
	ImGui::Begin("GameMode", nullptr, ImGuiWindowFlags_NoBackground);
	{
		ImVec2 wpos = ImGui::GetWindowPos();
		ImVec2 wsize = ImGui::GetWindowSize();
		GameInstance::GetInstance().RegisterUIBackgroundRect((int)wpos.x, (int)wpos.y, (int)wsize.x, (int)wsize.y);
	}
	ImGui::Text("GameState : %d", (int32)_state);
	ImGui::End();
}
