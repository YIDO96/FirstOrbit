#include "pch.h"
#include "LaunchGameMode.h"

#include "Core/TimeManager.h"
#include "Core/InputManager.h"

#include "Gameframework/Components/UPhysicsComponent.h"
#include "Actor/ASpaceship.h"

void LaunchGameMode::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (_launchState == ELaunchState::Ascent and _ship and _ship->GetCenterPos().y > 0.f)
	{
		ChangeLaunchState(ELaunchState::Failed);
	}
}

void LaunchGameMode::OnGUI()
{
	if (ImGui::TreeNode("GameMode"))
	{
		ImGui::Text("GameMode : LaunchGameMode");
		Super::OnGUI();
		switch (_launchState)
		{
		case ELaunchState::Idle:
			ImGui::Text("LaunchState: Idle");
			break;
		case ELaunchState::Countdown:
			ImGui::Text("LaunchState: Countdown");
			break;
		case ELaunchState::Ascent:
			ImGui::Text("LaunchState: Ascent");
			break;
		case ELaunchState::Failed:
			ImGui::Text("LaunchState: Failed");
			break;
		case ELaunchState::Success:
			ImGui::Text("LaunchState: Success");
			break;
		case ELaunchState::MAX_State:
			break;
		default:
			break;
		}

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
			}, 3.f);
	}
}

void LaunchGameMode::Reset()
{
	TIME.Remove(_countdownTimerId);   // id가 -1이어도 안전 (매칭 안 되면 그냥 무시됨)

	if (_ship)
		_ship->Reset();

	ChangeLaunchState(ELaunchState::Idle);
}
