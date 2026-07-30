#pragma once

#include "Singleton.h"



class World;

// 여러 World(레벨)를 등록해두고 전환한다 (≈ 언리얼의 레벨 전환/스트리밍)
class WorldManager : public Singleton<WorldManager>
{
	friend Singleton<WorldManager>;

	using WorldFactory = function<World*()>;

public:
	void RegisterWorld(const string& name, WorldFactory worldFactory);

	void ChangeWorld(const string& name);
	void Update(float deltaTime);
	void Render(HDC hdc);
	void OnGUI();

	World* GetWorld() const { return _currentWorld; }

private:
	// 아무나 생성못하게 생성자/소멸자를 숨기자
	WorldManager() = default;
	~WorldManager() = default;

	// 예약된 전환(_nextWorldName)을 실제로 적용한다 (delete/생성/Enter).
	// World::Update 콜스택 바깥(Update 진입 직전)에서만 호출해 use-after-free를 피한다.
	void ApplyWorldChange();

private:
	unordered_map<string, WorldFactory> _worldList;

	World* _currentWorld = nullptr;

	// 다음 프레임에 전환할 월드 이름. 비어있으면("") 예약된 전환이 없다는 뜻.
	string _nextWorldName;
};

#define WORLD WorldManager::GetInstance()
