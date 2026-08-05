#pragma once

#include "GameFramework/World.h"

class LaunchWorld : public World
{
	using Super = World;

public:
	virtual void Enter() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void LoadTexture() override;

	virtual void ResetCamera(Vector2 pos) { _camera.SetPosition(pos); _camera.SetZoomImmediate(2.f); }

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const { return L"LaunchWorld"; }

	virtual GameMode* CreateGameMode() override;

private:
	class ASpaceship* _ship = nullptr;
};
