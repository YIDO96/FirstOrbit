#pragma once

#include "GameFramework/World.h"
class AActor;

// 실제 게임플레이가 진행되는 씬.
// TODO: 공/패들/벽돌 액터가 생기면, G 키로 GameOverWorld 전환하는 데모 트리거는
//       지우고 실제 승패 조건(GameMode)으로 교체한다.
class MainWorld : public World
{
	using Super = World;

public:
	~MainWorld();


	virtual void Enter() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

	virtual void LoadTexture() override;
	virtual void SetCameraOnGUIDoubleClickedEvent(AActor* target) override;

	void InitPlanet();
	void UpdateUIData();
	vector<class APlanet*> GetReachablePlanets() const;   // 삥 도는 전이만 나오는 행성은 제외

	// 뷰포트 클릭이든(PickActor), 액터 버튼 클릭이든 전부 여기로 모여서 같은 로직을 탄다.
	// 우주선을 픽하면 "목적지 선택 모드"로 들어가고, 그 상태에서 행성을 고르면 전이가 시작된다.
	void SelectActor(AActor* actor);
	void StartTransfer(class APlanet* target);
	// 빈 공간 클릭 등으로 선택/픽 상태를 전부 해제한다 (버튼도 다시 전부 활성으로 되돌림).
	void DeselectActor();

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const { return L"MainWorld"; }

	virtual GameMode* CreateGameMode() override;

public:

private:
	class Texture* _bg = nullptr;

	vector<class APlanet*> _planets;

	AActor* _selected = nullptr;

	//class StarField* _starField = nullptr;
	class SpaceLightingSystem* _lightingSystem = nullptr;
	class APlanet* _sun = nullptr;

	class APlanet* _homePlanet = nullptr;
	class ASpaceship* _ship = nullptr;
	class ABlackHole* _blackHole = nullptr;

	class Widget_Main* _widget = nullptr;

	vector<Vector2> _transferPath;


	class APlanet* _transferTarget = nullptr;
	bool _transferCaptured = false;
	float _transferAngleDeg = 0.f;
	float _transferApoapsis = 0.f;

	float _lastDeltaTime = 0.f;
};
