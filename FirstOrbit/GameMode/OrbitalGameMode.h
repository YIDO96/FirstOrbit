#pragma once

#include "GameFramework/GameMode.h"

enum class EOrbitState
{
	Flying,
	Cleared,
	Failed,
};

struct OrbitState // 예측 전용
{
	Vector2 pos;
	Vector2 vel;
};

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

	void ResetJudgment() { _orbitState = EOrbitState::Flying; _graceTimer = 0.f; _isWarning = false; }

	vector<Vector2> PredictPath(int steps, float dt) const;

private:
	ASpaceship* _ship = nullptr;
	APlanet* _home = nullptr;

	EOrbitState _orbitState = EOrbitState::Flying;

	float _a = 0.f;				// ImGui에 표시할 임시 변수

	float _eps = 0.f;			// 비에너지(Specific Orbital Energy) : 궤도의 총 역학적 에너지 ( >= 0 이면 탈출 궤도)
	float _perigee = 0.f;		// 근지점 / 근점 거리 (Perigee / Periapsis) : 중심 천체와 가장 가까워지는 거
	float _apogee = 0.f;		// 원지점 / 원점 거리 (Apogee / Apoapsis) : 중심 천체와 가장 멀어지는 거리
	float _eccentricity = 0.f;	// 이심률 (Eccentricity) : 궤도의 일그러진 정도 (0 : 원 , 0 ~ 1 : 타원, 1 : 포물선, >1 : 쌍곡선) 
	bool _isEscaping = false;	// 탈출 여부 : 비에너지(_eps)가 0 이상이거나 이심율이 1이상일 때  true

	bool _isWarning = false;

	// 판정 미뤄두는 시간
	float _graceTimer = 0.f;
	static constexpr float kGracePeriod = 10.f;
};

