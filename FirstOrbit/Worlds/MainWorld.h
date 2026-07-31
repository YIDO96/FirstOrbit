#pragma once

#include "GameFramework/World.h"
class AActor;

// 실제 게임플레이가 진행되는 씬.
// TODO: 공/패들/벽돌 액터가 생기면, G 키로 GameOverWorld 전환하는 데모 트리거는
//       지우고 실제 승패 조건(GameMode)으로 교체한다.
class MainWorld : public World
{
	using Super = World;

public:
	~MainWorld();


	virtual void Enter() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

	virtual void LoadTexture() override;
	void InitPlanet();

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const { return L"MainWorld"; }

public:

private:
	class Texture* _bg = nullptr;

	AActor* _selected = nullptr;

	class APlanet* _homePlanet = nullptr;

	class StarField* _starField;
};
