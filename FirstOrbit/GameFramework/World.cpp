#include "pch.h"
#include "World.h"

#include "Core/GameInstance.h"
#include "Core/ResourceManager.h"
#include "Core/TimeManager.h"
#include "Core/DataManager.h"
#include "Core/ResourceData.h"
#include "Core/InputManager.h"

#include "GameFramework/Texture.h"
#include "GameFramework/AActor.h"
#include "GameFramework/Components/UColliderComponent.h"
#include "GameFramework/GameMode.h"

World::~World()
{
    delete _gameMode;

    for (AActor* a : _actors) delete a;
    _actors.clear();
}

GameMode* World::CreateGameMode()
{
    return new GameMode();
}

void World::Enter()
{
    // DataManager/ResourceData(JSON)에 정의된 텍스처를 먼저 전부 로드해둔다.
    // (아래에서 스폰되는 액터들의 Init()이 GetTexture()로 바로 꺼내 쓸 수 있어야 하므로 반드시 먼저 호출)
    loadResources();

    _gameMode = CreateGameMode();
    _gameMode->Init(this);

    _camera.Init(GWinSizeX, GWinSizeY);
}

void World::Update(float deltaTime)
{
    _camera.Update(deltaTime);

    for (AActor* actor : _actors)
    {
        if (actor->GetIsActive()) actor->Update(deltaTime);
    }


    for (AActor* actor : _actors)
        if (actor->GetIsActive()) actor->UpdateCollider(deltaTime);

    CheckOverlaps();
    ProcessPendingDestroy();

    if (_gameMode)
        _gameMode->Update(deltaTime);
}

void World::GetCellRangeForActor(AActor* actor, int32& minX, int32& minY, int32& maxX, int32& maxY) const
{
    AABB box = actor->GetCollider()->GetAABB();

    minX = std::clamp((int32)(box.min.x / GridCellSize), 0, GridCols - 1);
    minY = std::clamp((int32)(box.min.y / GridCellSize), 0, GridRows - 1);
    maxX = std::clamp((int32)(box.max.x / GridCellSize), 0, GridCols - 1);
    maxY = std::clamp((int32)(box.max.y / GridCellSize), 0, GridRows - 1);
}

void World::RebuildGrid()
{
    for (auto& cell : _grid)
        cell.clear();

    // 콜라이더를 가진 모든 활성 액터를 격자에 등록한다.
    for (AActor* actor : _actors)
    {
        if (!actor->GetIsActive() || !actor->GetCollider()) continue;

        int32 minX, minY, maxX, maxY;
        GetCellRangeForActor(actor, minX, minY, maxX, maxY);

        // (y, x) 이중 루프 대신, 걸친 칸 범위(가로 width칸 x 세로 height칸)를
        // 한 줄로 편 1차원 인덱스 하나로 순회한다.
        int32 width = maxX - minX + 1;
        int32 cellCount = width * (maxY - minY + 1);

        for (int32 i = 0; i < cellCount; ++i)
        {
            int32 x = minX + (i % width);
            int32 y = minY + (i / width);
            _grid[y * GridCols + x].push_back(actor);
        }
    }
}

void World::CollectGridCandidates(AActor* actor, unordered_set<AActor*>& out) const
{
    int32 minX, minY, maxX, maxY;
    GetCellRangeForActor(actor, minX, minY, maxX, maxY);

    int32 width = maxX - minX + 1;
    int32 cellCount = width * (maxY - minY + 1);

    for (int32 i = 0; i < cellCount; ++i)
    {
        int32 x = minX + (i % width);
        int32 y = minY + (i / width);

        for (AActor* other : _grid[y * GridCols + x])
            out.insert(other);
    }
}

void World::CheckOverlaps()
{
    // 언리얼처럼 액터 쌍마다 겹침 여부가 "바뀌는 순간"에만 Begin/EndOverlap을 쏜다.
    // 콜라이더를 가진 모든 활성 액터가 격자를 조회해 후보를 좁힌다 (게임 콘텐츠에 종속되지 않은 범용 브로드페이즈).
    RebuildGrid();

    for (AActor* actor : _actors)
    {
        if (!actor->GetIsActive() || !actor->GetCollider()) continue;

        unordered_set<AActor*> candidates;
        CollectGridCandidates(actor, candidates);

        for (AActor* other : candidates)
        {
            // CheckOverlap이 이미 양쪽(actor/other)에 이벤트를 쏘므로,
            // 포인터 주소 비교로 각 쌍을 한 번만 처리한다.
            if (other <= actor) continue;
            actor->GetCollider()->CheckOverlap(other);
        }
    }
}

void World::ProcessPendingDestroy()
{
    for (AActor* actor : _pendingDestroy)
    {
        // 삭제되기 전에, 이 액터와 겹쳐있던 다른 액터들에게 EndOverlap을 알려준다.
        for (AActor* other : _actors)
        {
            if (other == actor) continue;
            if (other->GetCollider() && other->GetCollider()->IsOverlapping(actor))
                other->GetCollider()->EndOverlap(actor);
        }

        _actors.erase(std::remove(_actors.begin(), _actors.end(), actor), _actors.end());

        // SpawnPooledActor()로 태어난 액터는 delete 대신 풀에 돌려준다 (재사용).
        if (IObjectPool* pool = actor->GetPool())
        {
            actor->SetActive(false);
            pool->Return(actor);
        }
        else
        {
            delete actor;
        }
    }
    _pendingDestroy.clear();
}
AActor* World::PickActor(Vector2 worldPos) const
{
    for (AActor* actor : _actors)
    {
        UColliderComponent* col = actor->GetCollider();
        if (!col || col->GetColliderType() != ColliderType::Circle) continue;

        if ((worldPos - actor->GetCenterPos()).Length() <= col->GetRadius())
            return actor;
    }

    return nullptr;
}
void World::Render(HDC hdc)
{
    for (AActor* actor : _actors)
        if (actor->GetIsActive()) actor->Render(hdc);

    if (GameInstance::GetInstance().IsDebugMode())
    {
        //RenderDebugGrid(hdc);
    }
}

void World::RenderDebugGrid(HDC hdc)
{
    HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, ::GetStockObject(HOLLOW_BRUSH));

    // 1. 전체 격자: 빨간색
    HPEN redPen = ::CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    HPEN oldPen = (HPEN)::SelectObject(hdc, redPen);

    for (int32 y = 0; y < GridRows; ++y)
        for (int32 x = 0; x < GridCols; ++x)
            Rectangle(hdc, x * GridCellSize, y * GridCellSize, (x + 1) * GridCellSize, (y + 1) * GridCellSize);

    // 2. 콜라이더를 가진 액터가 이번 프레임에 실제로 조회한 칸: 초록색으로 덧그린다.
    HPEN greenPen = ::CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
    ::SelectObject(hdc, greenPen);

    for (AActor* actor : _actors)
    {
        if (!actor->GetIsActive() || !actor->GetCollider()) continue;

        int32 minX, minY, maxX, maxY;
        GetCellRangeForActor(actor, minX, minY, maxX, maxY);

        int32 width = maxX - minX + 1;
        int32 cellCount = width * (maxY - minY + 1);

        for (int32 i = 0; i < cellCount; ++i)
        {
            int32 x = minX + (i % width);
            int32 y = minY + (i / width);
            Rectangle(hdc, x * GridCellSize, y * GridCellSize, (x + 1) * GridCellSize, (y + 1) * GridCellSize);
        }
    }

    ::SelectObject(hdc, oldPen);
    ::SelectObject(hdc, oldBrush);
    ::DeleteObject(redPen);
    ::DeleteObject(greenPen);
}

void World::loadResources()
{
    // DataManager에 가서 이 World의 섹션(GetResourceSectionName())에 해당하는 텍스처를 요청
    ResourceData* data = DataManager::GetInstance().FindData<ResourceData>(L"ResourceData");
    if (!data) return;

    // 사운드는 씬 구분 없이(SoundManager::Play가 어느 World에서든 키로 바로 재생할 수 있어야 하므로)
    // 전부 미리 로드해둔다. LoadSound는 이미 로드된 키를 무시하므로 중복 호출해도 안전하다.
    for (auto& iter : data->GetSoundSection())
    {
        const ResourceData::SoundItem& item = iter.second;
        ResourceManager::GetInstance().LoadSound(iter.first, item.fileName);
    }

    const ResourceData::Section* section = data->FindSection(GetResourceSectionName());
    if (!section) return;

    // 새로운 텍스처 로드가 필요하면, json에 추가하면 끝!
    for (auto& iter : *section)
    {
        const ResourceData::Item& item = iter.second;
        ResourceManager::GetInstance().LoadTexture(
            iter.first,
            item.fileName,
            item.transparent,
            item.countY,
            item.countX,
            item.dur);
    }
}

void World::Exit()
{
}

void World::LoadTexture()
{
}

void World::OnGUI()
{
    //ImGui::Begin(_worldName.empty() ? "World" : _worldName.c_str());
    ImGui::Begin("World");
    ImGui::Text("World : %s", _worldName.c_str());
    OnSceneGUI();


    ImGui::End();
    if (_gameMode)
        _gameMode->OnGUI();



    ImGui::Begin("World Hierarchy");
    
    _camera.OnSceneGUI();
    if (_actors.size() >= 1)
    {
        if (ImGui::TreeNode("Actor"))
        {
            for (AActor* actor : _actors)
                if (actor->GetIsActive()) actor->OnGUI();

            ImGui::TreePop();
        }
    }
   
    ImGui::End();

    
}



void World::DestroyActor(AActor* actor)
{
    if (!actor) return;
    if (std::find(_pendingDestroy.begin(), _pendingDestroy.end(), actor) != _pendingDestroy.end())
        return; // 이미 삭제 대기 중

    _pendingDestroy.push_back(actor);
}
