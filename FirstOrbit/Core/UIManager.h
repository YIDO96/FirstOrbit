#pragma once

#include "Singleton.h"

// 인게임 UI(HP 등) 렌더링을 전담한다. GameInstance::Render()가 World::Render() 직후
// 같은 게임 해상도 버퍼(_hdcGame)에 이어서 그려 넣는다.
class UIManager : public Singleton<UIManager>
{
	// Singleton 객체를 '친구'로 선언해서 private 접근 가능하게 열어준다.
	friend Singleton<UIManager>;

public:
	void Init();
	void Update(float deltaTime);
	void Render(HDC hdc);

private:
	// 아무나 생성못하게 생성자/소멸자를 숨기자
	UIManager() = default;
	~UIManager() = default;

private:
	class Texture* _hpTexture = nullptr;
};

#define UI UIManager::GetInstance()
