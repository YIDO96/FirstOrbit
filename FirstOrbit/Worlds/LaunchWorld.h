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
	COLORREF ComputeRayleighSkyColor(float density);

	class Widget_Launch* GetWidget()  { return _launchWidget; }

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const { return L"LaunchWorld"; }

	virtual GameMode* CreateGameMode() override;

private:
	class ASpaceship* _ship = nullptr;

	class Widget_Launch* _launchWidget = nullptr;

	float _altitude = 0.f;

	static constexpr float kSpaceAltitude = 10000.f;

	class StarField* _stars = nullptr;
};
