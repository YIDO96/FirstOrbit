#include "pch.h"
#include "Widget_Title.h"

#include "Core/ResourceManager.h"
#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/SoundManager.h"

#include "GameFramework/UI/UIText.h"
#include "GameFramework/UI/UIButton.h"
#include "GameFramework/Texture.h"


void Widget_Title::Init()
{
	Super::Init();

	// 이 위젯의 자식들은 전부 오른쪽 사이드 패널(GImGuiPanelWidth 기준) 좌표계에 그려진다.
	// (UIManager가 이제 항상 패널 버퍼에만 그리기 때문에, 여기서 안 켜주면 게임 뷰포트 기준
	// 좌표로 계산된 버튼이 패널 버퍼 밖으로 밀려나면서 클릭 판정도 게임 화면 쪽으로 어긋난다.)
	UIBase::SetActiveUISpace(true);

	_titleName = AddChild<UIText>();
	_titleName->Init(EAnchor::Top, EPivot::Top, Vector2(0, 80.f), Vector2(0, 0), L"FirstOrbit");
	_titleName->SetFontSize(36.f);

	Texture* buttonTex = RESOURCE.GetTexture(L"ButtonBG");

	_startButton = AddChild<UIButton>();
	_startButton->SetTexture(buttonTex);
	Vector2 startButtonSize = buttonTex->GetTextureSize() * 1.3f;
	_startButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, -40.f), startButtonSize);
	_startButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 0), startButtonSize, L"Start");
	_startButton->SetOnClick([]()
		{
			WORLD.ChangeWorld("LaunchWorld");
			SOUND.Stop(L"S_Main");
		});

	_exitButton = AddChild<UIButton>();
	_exitButton->SetTexture(buttonTex);
	_exitButton->Init(EAnchor::Center, EPivot::Center, Vector2(0, 40.f), startButtonSize);
	_exitButton->SetText(EParentAnchor::Center, EPivot::Center, Vector2(0, 0), startButtonSize, L"Exit");
	_exitButton->SetOnClick([]()
		{
			PostQuitMessage(0);
		});

	UIBase::SetActiveUISpace(false);   // 원상 복구 (다음에 만들어질 다른 UI가 영향받지 않도록)
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
