#include "pch.h"
#include "TitleWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/UIManager.h"
#include "Widget/Widget_Title.h"

void TitleWorld::Enter()
{
	Super::Enter();

	LoadTexture();

	_camera.SetIsControll(false);
	_titleWidget = UI.CreateWidget<Widget_Title>();
	_titleWidget->SetOwnerWorld(this);
}

void TitleWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (_INPUT.GetButtonDown(KeyType::P))
		WORLD.ChangeWorld("LaunchWorld");

	//if (_INPUT.GetButtonDown(KeyType::F1))
	//	WORLD.ChangeWorld("EditorWorld");
}

void TitleWorld::OnSceneGUI()
{
	//ImGui::Text("F1 : EditorWorld로 진입");
	ImGui::Text("P : 게임 시작");

	if (ImGui::Button("Widget Editor"))
	{
		_titleWidget->SetActiveEidtMode(_isWidgetEditToggle);

		_isWidgetEditToggle = !_isWidgetEditToggle;
	}

}
