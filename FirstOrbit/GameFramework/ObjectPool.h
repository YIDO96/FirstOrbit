#pragma once

// ObjectPool<T>는 템플릿이라 타입별로 서로 다른 클래스가 되므로, AActor는
// 자신이 어떤 풀에서 나왔는지를 이 인터페이스 하나로만 기억해두면 된다.
class IObjectPool
{
public:
	virtual ~IObjectPool() = default;
	virtual void Return(class AActor* actor) = 0;
};

// AActor 파생 타입 T를 미리 size개만큼 생성해두고 재사용하는 풀.
// World::SpawnPooledActor()로 꺼내 쓰고, World::DestroyActor()로 반납하면
// new/delete 대신 이 풀을 왕복한다 (자주 생성/소멸되는 이펙트 등에 사용).
template<typename T>
class ObjectPool : public IObjectPool
{
public:
	void Init(int32 size)
	{
		_buffer.resize(size);
		_freeList.reserve(size);

		for (auto& item : _buffer)
		{
			AActor* actor = static_cast<AActor*>(&item);
			actor->SetPool(this);
			_freeList.push_back(&item);
		}
	}

	T* Acquire()
	{
		if (_freeList.empty())
		{
			// 풀이 부족하면 Init(size)의 크기를 늘려서 해결한다 (런타임 재할당은 하지 않음).
			assert(false && "ObjectPool exhausted: increase Init(size)");
			return nullptr;
		}

		T* object = _freeList.back();
		_freeList.pop_back();
		return object;
	}

	virtual void Return(AActor* actor) override
	{
		_freeList.push_back(static_cast<T*>(actor));
	}

private:
	vector<T>  _buffer;	// 실제 메모리를 소유
	vector<T*> _freeList;	// 지금 대여 가능한 것들의 주소
};
