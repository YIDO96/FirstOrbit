#pragma once

#include "GameFramework/World.h"


// 에디터 화면 씬. (나중에 여기서 Ball 배치, Star 배치 등 에디터 기능 추가 예정)
class EditorWorld : public World
{
	using Super = World;

public:
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const override { return L"EditorScene"; }
};
