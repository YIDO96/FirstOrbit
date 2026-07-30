#pragma once

#include "GameFramework/World.h"

// 게임 오버 화면 씬. SpaceBar를 누르면 TitleWorld로 되돌아간다.
class GameOverWorld : public World
{
	using Super = World;

public:
	virtual void Update(float deltaTime) override;

protected:
	virtual void OnSceneGUI() override;
};
