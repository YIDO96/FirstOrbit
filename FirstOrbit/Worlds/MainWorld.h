#pragma once

#include "GameFramework/World.h"
#include "GameFramework/Camera.h"


// 실제 게임플레이가 진행되는 씬.
// TODO: 공/패들/벽돌 액터가 생기면, G 키로 GameOverWorld 전환하는 데모 트리거는
//       지우고 실제 승패 조건(GameMode)으로 교체한다.
class MainWorld : public World
{
	using Super = World;

public:

	virtual void Enter() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

	virtual void LoadTexture() override;

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const { return L"MainWorld"; }

public:
	Camera& GetCamera() { return _camera; }

private:
	Camera _camera;
	class Texture* _bg = nullptr;

	class Texture* _bgs[25];
};
