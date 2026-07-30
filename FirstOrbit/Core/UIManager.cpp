#include "pch.h"
#include "UIManager.h"
#include "ResourceManager.h"
#include "GameFramework/Texture.h"

void UIManager::Init()
{
	_hpTexture = ResourceManager::GetInstance().GetTexture(L"PlayerHP");
}

void UIManager::Update(float deltaTime)
{
}

void UIManager::Render(HDC hdc)
{
	if (_hpTexture == nullptr)
		return;

	// TODO: HP 등 인게임 UI를 그릴 액터/데이터가 생기면 여기서 실제로 그린다.
}
