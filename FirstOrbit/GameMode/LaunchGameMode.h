#pragma once

#include "GameFramework/GameMode.h"


enum class ELaunchState
{
	Idle,			// 발사 대기
	Countdown,		// 카운트 다운
	Ascent,			// 상승 중
	Failed,			// 지면 충돌
	Success,		// 궤도 진입

	MAX_State
};

class LaunchGameMode : public GameMode
{
	using Super = GameMode;
public :
	virtual void Init(World* world) { _world = world; }
	virtual void Update(float deltaTime) override;
	virtual void OnGUI() override;

	void ChangeLaunchState(ELaunchState newState);

	void Reset();

	void SetShip(class ASpaceship* ship) { _ship = ship; }
	ELaunchState GetLaunchState() const { return _launchState; }

private:
	class ASpaceship* _ship = nullptr;

	int32 _countdownTimerId = -1;

	ELaunchState _launchState = ELaunchState::Idle;
	vector<string> stateName = { "Idle", "Countdown", "Ascent", "Failed", "Success" };
};

