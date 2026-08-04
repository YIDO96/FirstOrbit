#pragma once

#include "GameFramework/GameMode.h"

class ASpaceship;
class APlanet;

class OrbitalGameMode : public GameMode
{
	using Super = GameMode;
public:
	virtual void Init(World* world) { _world = world; }
	virtual void Update(float deltaTime) override;
	virtual void OnGUI() override;

	void SetShip(ASpaceship* ship) { _ship = ship; }

private:
	ASpaceship* _ship = nullptr;
	APlanet* _home = nullptr;
};

