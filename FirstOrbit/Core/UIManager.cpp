#include "pch.h"
#include "UIManager.h"
#include "ResourceManager.h"
#include "GameFramework/Texture.h"

#include "GameFramework/UI/Widget.h"

void UIManager::Init()
{
}

void UIManager::Update(float deltaTime)
{
	for (auto* widget : _widgets)
	{
		widget->Update(deltaTime);
	}
}

void UIManager::Render(HDC hdc)
{
	for (auto* widget : _widgets)
	{
		widget->Render(hdc);
	}
	// TODO: HP 등 인게임 UI를 그릴 액터/데이터가 생기면 여기서 실제로 그린다.
}

void UIManager::Clear()
{
	for (auto* widget : _widgets)
		delete widget;
	_widgets.clear();
}

bool UIManager::IsEidtMode()
{
	bool editmode = false;

	for (auto* widget : _widgets)
	{
		if (not widget->GetIsEditMode()) continue;

		editmode = true;
	}


	return editmode;
}

void UIManager::OnGUI()
{
	for (auto* widget : _widgets)
	{
		widget->OnGUI();
	}
}
