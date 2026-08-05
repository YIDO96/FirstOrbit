#include "pch.h"
#include "Widget_Title.h"

#include "GameFramework/UI/UIText.h"

void Widget_Title::Init()
{
	_titleName = AddChild<UIText>();
	_titleName->SetName(L"Text_FirstOrbit");
	_titleName->SetPos(0, 0);
}

void Widget_Title::Update(float deltaTime)
{
	_titleName->Update(deltaTime);
}

void Widget_Title::Render(HDC hdc)
{
	_titleName->Render(hdc);
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
