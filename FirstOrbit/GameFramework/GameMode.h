#pragma once

enum class EGameState
{
	Playing,
	GameOver
};

class World;

// 게임 규칙(승패 판정, 스폰 규칙, 점수 등)을 전담하는 클래스 (≈ 언리얼의 AGameModeBase).
// World가 하나 소유하고 매 프레임 위임한다. 실제 게임 규칙은 이 클래스를 상속해서 채운다.
class GameMode
{
public:
	virtual ~GameMode() = default;

	virtual void Init(World* world) { _world = world; }
	virtual void Update(float deltaTime) {}
	virtual void OnGUI();

	void SetGameOver() { _state = EGameState::GameOver; }
	bool IsGameOver() const { return _state == EGameState::GameOver; }
	EGameState GetState() const { return _state; }

protected:
	World* _world = nullptr;
	EGameState _state = EGameState::Playing;
};
