#pragma once

#include "ObjectPool.h"

class AActor;
class GameMode;

// 액터 컨테이너 + 충돌 브로드페이즈 (≈ 언리얼의 UWorld).
// 배경/스폰 같은 콘텐츠 로직은 여기 두지 않고, 게임 규칙은 GameMode에 위임한다.
class World
{
public:
    World() = default;
	virtual ~World();

    virtual void Enter();
	virtual void Update(float deltaTime);
	virtual void Render(HDC hdc);
    virtual void Exit();

    virtual void LoadTexture();
    virtual void OnGUI();

    void SetWorldName(string name) { _worldName = name; }
    string GetWorldName() const { return _worldName; }

    // 액터 스폰 (≈ UWorld::SpawnActor)
    template<typename T>
    T* SpawnActor()
    {
        T* actor = new T();
        actor->SetOwnerWorld(this);
        actor->Init();
        _actors.push_back(actor);
        return actor;
    }
    // 오브젝트 풀에서 액터를 빌려와 스폰한다 (≈ SpawnActor와 동일하지만 new 대신 풀을 재사용).
    // 자주 생성/소멸되는 액터(이펙트 등)는 이 쪽을 쓰면 DestroyActor()가 자동으로 풀에 반납한다.
    template<typename T>
    T* SpawnPooledActor(ObjectPool<T>& pool)
    {
        T* actor = pool.Acquire();
        if (!actor) return nullptr;

        actor->SetOwnerWorld(this);
        actor->SetActive(true);
        actor->Init();
        _actors.push_back(actor);
        return actor;
    }

    // 즉시 삭제하지 않고 프레임 끝에서 일괄 삭제한다.
    // (Overlap 콜백이 순회 중인 _actors를 직접 건드리면 반복자가 깨지므로)
    void DestroyActor(AActor* actor);

    AActor* GetActor(int idx) const { return _actors[idx]; }
    int32 GetActorCount() const { return _actors.size(); }

    GameMode* GetGameMode() const { return _gameMode; }

protected:
    vector<AActor*> _actors;
    string _worldName;

    // 이 World가 사용할 GameMode를 생성한다. 게임별 규칙은 이 함수를 오버라이드해서 교체한다.
    virtual GameMode* CreateGameMode();
    GameMode* _gameMode = nullptr;

    // World::OnGUI()가 _worldName 창을 열어둔 상태에서 호출해준다.
    // 씬별 UI는 OnGUI()를 직접 오버라이드하지 말고 이걸 오버라이드해서 채운다
    // (Begin()을 두 번 호출하지 않기 위함).
    virtual void OnSceneGUI() {}

    // ResourceData.json에서 이 World가 읽어올 섹션 이름 (기본값 "TileWorld").
    // Title/Editor처럼 다른 텍스처 세트가 필요한 씬은 오버라이드해서 바꾼다.
    virtual wstring GetResourceSectionName() const { return L"TileWorld"; }

    void CheckOverlaps();
    void ProcessPendingDestroy();

    vector<AActor*> _pendingDestroy;



private:
    // ---- 공간 분할 격자 ----
    // 콜라이더를 가진 모든 활성 액터를 후보로 등록/조회하는 범용 브로드페이즈.
    // 셀 크기는 화면(GWinSizeX x GWinSizeY)에 정확히 나눠떨어지는 80으로 잡는다.
    static constexpr int32 GridCellSize = 80;
    static constexpr int32 GridCols = GWinSizeX / GridCellSize;
    static constexpr int32 GridRows = GWinSizeY / GridCellSize;

    vector<AActor*> _grid[GridCols * GridRows];

    void RebuildGrid();
    void GetCellRangeForActor(AActor* actor, int32& minX, int32& minY, int32& maxX, int32& maxY) const;
    void CollectGridCandidates(AActor* actor, unordered_set<AActor*>& out) const;

    // F1 디버그 모드에서 격자를 시각화한다. 전체 격자는 빨간색,
    // 콜라이더를 가진 액터가 이번 프레임에 실제로 조회한 칸은 초록색으로 덧그린다.
    void RenderDebugGrid(HDC hdc);

    void loadResources();
};
