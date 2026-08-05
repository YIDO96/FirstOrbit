#pragma once

#include "GameFramework/World.h"

// 타이틀 화면 씬. SpaceBar를 누르면 MainWorld로 전환한다.
class TitleWorld : public World
{
	using Super = World;

public:
	virtual void Enter() override;
	virtual void Update(float deltaTime) override;

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const override { return L"TitleWorld"; }

private:
	class Widget_Title* _titleWidget = nullptr;
};
