#include "pch.h"
#include "LaunchGameMode.h"

#include "Core/TimeManager.h"
#include "Core/InputManager.h"
#include "Core/WorldManager.h"

#include "Gameframework/Components/UPhysicsComponent.h"
#include "Actor/ASpaceship.h"
#include "Worlds/LaunchWorld.h"
#include "Widget/Widget_Launch.h"

void LaunchGameMode::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (_ship)
	{
		_altitude = -_ship->GetCenterPos().y;
	}

	if (_launchState == ELaunchState::Ascent and _ship and _ship->GetCenterPos().y > 0.f)
	{
		ChangeLaunchState(ELaunchState::Failed);
	}

	if (_launchState == ELaunchState::Countdown)
	{
		_countdownTimer -= deltaTime;
		str = to_string((int32)_countdownTimer + 1);
		UpdateTextByState();
	}
}

void LaunchGameMode::OnGUI()
{
	if (ImGui::TreeNode("GameMode"))
	{
		ImGui::Text("GameMode : LaunchGameMode");
		Super::OnGUI();

		const char* stateNames[] = { "Idle", "Countdown", "Ascent", "Failed"};
		ImGui::Text("LaunchState : %s", stateNames[(int32)_launchState]);

		ImGui::TreePop();
	}
}

void LaunchGameMode::ChangeLaunchState(ELaunchState newState)
{
	_launchState = newState;


	if (_ship)
	{
		bool shouldPause = (newState != ELaunchState::Ascent);
		//bool shouldPause = (newState == ELaunchState::Idle or newState == ELaunchState::Countdown);
		//bool shouldPause = (newState == ELaunchState::Ascent) and (_ship->GetKeyInput() == KeyType::W or _ship->GetKeyInput() == KeyType::Up);
		_ship->GetComponent<UPhysicsComponent>()->SetPaused(shouldPause);
	}


	if (newState == ELaunchState::Countdown)
	{
		_countdownTimerId  = TIME.AddTimer([this]()
			{
				ChangeLaunchState(ELaunchState::Ascent);
				_countdownTimer = 3.f;
			}, _countdownTimer);
	}
	UpdateTextByState();
}

void LaunchGameMode::UpdateTextByState()
{
	if (not _world) return;
	LaunchWorld* world = static_cast<LaunchWorld*>(_world);

	int32 timer = (int32)_countdownTimer;


	const char* stateNames[] = { "Awaiting Launch", str.c_str(), "Launching", "Failed" , "", ""};
	std::wstring text = CharToWStringStandard(stateNames[(int32)_launchState]);


	world->GetWidget()->SetStateText(text);
	world->GetWidget()->ReSizeStateText();
}

void LaunchGameMode::Reset()
{
	TIME.Remove(_countdownTimerId);   // id가 -1이어도 안전 (매칭 안 되면 그냥 무시됨)

	if (_ship)
		_ship->Reset();

	ChangeLaunchState(ELaunchState::Idle);
}

Vector2 LaunchGameMode::GetHorizentalVelocity() const
{
	return _ship->GetComponent<UPhysicsComponent>()->GetVelocity();
}
