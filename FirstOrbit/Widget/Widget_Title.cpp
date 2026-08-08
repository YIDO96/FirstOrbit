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
	Super::Init();

	_titleName = AddChild<UIText>();
	_titleName->SetText(L"FirstOrbit");
	_titleName->Init(EAnchor::Top, EPivot::Center, Vector2(0, 100), Vector2(100, 30));
	_titleName->SetFontSize(60.f);

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");

	_startButton = AddChild<UIButton>();
	_startButton->SetTexture(buttonTex);
	Vector2 startButtonSize = buttonTex->GetTextureSize() * 1.5f;
	_startButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, 0), startButtonSize);
	_startButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 0), startButtonSize, L"Start");
	_startButton->SetOnClick([]()
		{
			WORLD.ChangeWorld("LaunchWorld");
		});

	_exitButton = AddChild<UIButton>();
	_exitButton->SetTexture(buttonTex);
	_exitButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, 100), buttonTex->GetTextureSize() * 1.5f);
	_exitButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 0), startButtonSize, L"Exit");
	_exitButton->SetOnClick([]()
		{
			PostQuitMessage(0);
		});
}

void Widget_Title::Update(float deltaTime)
{
	Super::Update(deltaTime);

}

void Widget_Title::Render(HDC hdc)
{
	Super::Render(hdc);
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
