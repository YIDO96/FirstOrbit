#include "pch.h"
#include "WorldManager.h"

#include "GameFramework/World.h"


void WorldManager::RegisterWorld(const string& name, WorldFactory worldFactory)
{
	_worldList[name] = worldFactory;
}

void WorldManager::ChangeWorld(const string& name)
{
	// 등록되지 않은 월드로의 전환은 개발 단계에서 즉시 잡는다.
	assert(_worldList.find(name) != _worldList.end());

	_nextWorldName = name;

	// 첫 부팅(현재 월드가 아직 없음)은 지연할 이유가 없다.
	// 오히려 GameInstance::Init이 이 직후 UIManager를 초기화하며 첫 월드가 로드한
	// 텍스처를 참조하므로, 이 경우에 한해 즉시 적용해 시작 순서를 보장한다.
	if (_currentWorld == nullptr)
		ApplyWorldChange();

	// 인게임 전환은 여기서 아무것도 하지 않고 예약만 한다.
	// 실제 delete/생성은 다음 Update 진입 시 ApplyWorldChange가 수행한다.
	// (실행 중인 월드를 그 자리에서 delete하면, World::Update 콜스택에 살아있는
	//  this가 파괴돼 use-after-free가 된다.)
}

void WorldManager::ApplyWorldChange()
{
	if (_nextWorldName.empty())
		return;

	auto it = _worldList.find(_nextWorldName);
	assert(it != _worldList.end());

	if (_currentWorld)
		delete _currentWorld;

	_currentWorld = it->second();
	_currentWorld->SetWorldName(_nextWorldName);
	_currentWorld->Enter();

	// 전환이 적용되었으니 예약을 비운다.
	_nextWorldName.clear();
}

void WorldManager::Update(float deltaTime)
{
	// 예약된 전환을 현재 월드 Update '이전에' 적용한다 (프레임 경계에서 안전하게 교체).
	ApplyWorldChange();

	if (_currentWorld)
		_currentWorld->Update(deltaTime);
}

void WorldManager::Render(HDC hdc)
{
	if (_currentWorld)
		_currentWorld->Render(hdc);
}

void WorldManager::OnGUI()
{
	if (_currentWorld)
		_currentWorld->OnGUI();
}
