#include "pch.h"
#include "Widget_Title.h"

#include "Core/ResourceManager.h"
#include "Core/InputManager.h"
#include "Core/WorldManager.h"

#include "GameFramework/UI/UIText.h"
#include "GameFramework/UI/UIButton.h"
#include "GameFramework/Texture.h"


void Widget_Title::Init()
{
	_titleName = AddChild<UIText>();
	_titleName->SetText(L"FirstOrbit");
	_titleName->Init(EAnchor::Top, EPivot::Center, Vector2(0, 100), Vector2(100, 30));
	_titleName->SetFontSize(60.f);

	Texture* startTex = RESOURCE.GetTexture(L"StartButton");

	_startButton = AddChild<UIButton>();
	_startButton->SetTexture(startTex);
	_startButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, 0), startTex->GetTextureSize() * 1.5f);
	_startButton->SetOnClick([]()
		{
			WORLD.ChangeWorld("LaunchWorld");
		});


	Texture* exitTex = RESOURCE.GetTexture(L"ExitButton");
	_exitButton = AddChild<UIButton>();
	_exitButton->SetTexture(exitTex);
	_exitButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, 100), exitTex->GetTextureSize() * 1.5f);
	_exitButton->SetOnClick([]()
		{
			PostQuitMessage(0);
		});
}

void Widget_Title::Update(float deltaTime)
{
	//_titleName->Update(deltaTime);
	//_startButton->Update(deltaTime);
	for (auto* child : _children)
	{
		child->Update(deltaTime);
	}
}

void Widget_Title::Render(HDC hdc)
{
	//_titleName->Render(hdc);
	//_startButton->Render(hdc);
	for (auto* child : _children)
	{
		child->Render(hdc);
	}
}

void Widget_Title::OnGUI()
{
	if (ImGui::TreeNode("Widget_Title"))
	{
		for (auto* child : _children)
		{
			std::string name = child->GetName();

			if (ImGui::TreeNode(child, "%s", name.c_str()))
			{
				child->OnGUI();

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	
}
