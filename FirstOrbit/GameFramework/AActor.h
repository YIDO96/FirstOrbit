#pragma once


// 게임플레이 액터가 추가되면 여기에 태그를 확장해서 쓴다.
enum class EActorType
{
	None,
};

class World;
class UActorComponent;
class UColliderComponent;
class IObjectPool;

class AActor
{
public:
	AActor() = default;
	virtual ~AActor();

	virtual void Init() {}
	virtual void Update(float deltaTime);
	virtual void UpdateCollider(float deltaTime);
	virtual void Render(HDC hdc);
	virtual void OnGUI();

	// 컴포넌트 부착 (≈ 언리얼의 CreateDefaultSubobject/AddComponent).
	// new T(this, args...) 로 생성해 리스트에 넣고 Init()까지 호출한 뒤 반환한다.
	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		T* comp = new T(this, std::forward<Args>(args)...);
		_components.push_back(comp);
		comp->Init();
		return comp;
	}

	// 부착된 컴포넌트 중 T 타입으로 캐스팅되는 첫 번째를 반환한다 (없으면 nullptr).
	template<typename T>
	T* GetComponent() const
	{
		for (UActorComponent* c : _components)
			if (T* t = dynamic_cast<T*>(c))
				return t;
		return nullptr;
	}


	// ---- Transform 설정 ----
	// 내부 표준: _pos = 좌상단, _center = 중심(= _pos + _size/2). 이 규칙은 Update()에서 매 프레임 유지된다.
	void SetPos(Vector2 pos)			{ _pos = pos; }
	// 중심 좌표로 배치한다 (콜라이더들이 GetCenterPos()를 '중심'으로 사용하므로 규칙을 맞춘다).
	void SetCenterPos(Vector2 centerPos){ _pos = Vector2(centerPos.x - _size.x / 2, centerPos.y - _size.y / 2); _center = centerPos; }

	// 액터의 회전을 '한 번에' 설정한다. _degree/_forwardDir/_rightDir를 함께 갱신해
	// 셋이 항상 일치하도록 만드는 단일 진입점이다 (개별 필드를 직접 건드리지 말 것).
	void SetRotation(float degree);
	// 현재 각도에서 상대적으로 회전시킨다 (예: 좌우 키로 기체 기울이기).
	void AddRotation(float deltaDegree) { SetRotation(_degree + deltaDegree); }

	void SetActive(bool isActive)		{ _isActive = isActive; }
	void SetOwnerWorld(World* world)	{ _ownerWorld = world; }

	// ObjectPool<T>::Init()이 버퍼를 만들면서 채워준다. 풀에서 태어난 액터가 아니면 nullptr로 남는다.
	void SetPool(IObjectPool* pool)	{ _pool = pool; }
	IObjectPool* GetPool() const		{ return _pool; }

	// ---- Get ----
	Vector2 GetPos()			const { return _pos; }
	Vector2 GetCenterPos()		const { return _center; }
	Vector2 GetSize()			const { return _size; }
	UColliderComponent* GetCollider()	const { return _collider; }
	Vector2 GetAbsoluteUpDir()	const { return Vector2(0, -1); }			// 화면 절대 위쪽(회전 무관)
	Vector2 GetForwardDir()		const { return _forwardDir; }				// 정면 방향(회전 반영)
	Vector2 GetRightDir()		const { return _rightDir; }					// 오른쪽 방향(회전 반영)
	EActorType GetType()		const { return _type; }
	bool GetIsActive()			const { return _isActive; }
	float GetDegree()			const { return _degree; }

protected:
	World* _ownerWorld			= nullptr;
	EActorType _type			= EActorType::None;

	// 부착된 모든 컴포넌트 (소유). Update/Render/OnGUI에서 전부 순회 위임한다.
	vector<UActorComponent*> _components;

	// World의 충돌 브로드페이즈가 매 프레임 핫패스에서 콜라이더를 조회하므로,
	// GetComponent<T>()의 dynamic_cast 탐색 대신 언리얼의 RootComponent처럼 직접 캐시해둔다.
	// 파생 액터의 Init()에서 `_collider = AddComponent<UColliderComponent>(...)`로 채운다.
	UColliderComponent* _collider = nullptr;

	// null이 아니면 World가 delete 대신 pool->Return(this)로 반납한다.
	IObjectPool* _pool			= nullptr;

	// ---- Transform (표준: _pos=좌상단, _center=중심, _degree가 회전의 단일 소스) ----
	Vector2 _pos				= Vector2();
	Vector2 _center				= Vector2();
	Vector2 _size				= Vector2();

	// 회전 상태. _degree만 SetRotation으로 바꾸면 두 방향 벡터가 그로부터 파생된다.
	float   _degree				= 0.f;
	Vector2 _forwardDir			= Vector2(0, -1);	// 정면(0도 기준 = 화면 위)
	Vector2 _rightDir			= Vector2(1,  0);	// 오른쪽(0도 기준)

	bool _isActive				= true;
};
