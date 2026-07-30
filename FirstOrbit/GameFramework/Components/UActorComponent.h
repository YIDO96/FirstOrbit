#pragma once

class AActor;

// 액터에 붙는 재사용 가능한 기능 단위 (≈ 언리얼의 UActorComponent).
// AActor는 이 컴포넌트들의 목록을 들고 매 프레임 Update/Render/OnGUI를 위임한다.
class UActorComponent
{
public:
	UActorComponent(AActor* owner) : _owner(owner) {}
	virtual ~UActorComponent() = default;

	virtual void Init() {}
	virtual void Update(float deltaTime) {}
	virtual void Render(HDC hdc) {}
	virtual void OnGUI() {}

	AActor* GetOwner() const { return _owner; }

protected:
	AActor* _owner = nullptr;
};
