#pragma once

#include "GameFramework/AActor.h"

class ABlackHole : public AActor
{
	using Super = AActor;

public:
	virtual void Init() override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;

	void Setup(Vector2 pos, float mu, float eventHorizonRadius);

	float GetMu() const { return _mu; }
	float GetEventHorizonRadius() const { return _eventHorizonRadius; }

private:
	float _mu = 0.f;
	float _eventHorizonRadius = 0.f;
};

