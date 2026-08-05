#pragma once

#include "GameFramework/UIBase.h"

class Widget : public UIBase
{
	using Super = UIBase;

public:
	Widget() = default;
	virtual ~Widget();

	// 자식 UI 추가
	template <typename T, typename... Args>
	T* AddChild(Args&&... args)
	{
		T* child = new T(std::forward<Args>(args)...);
		_children.push_back(child);
		
		return child;
	}

	virtual void Init();

	virtual void Update(float deltaTime);

	virtual void Render(HDC hdc);

	virtual void OnGUI() {}

	void ActiveEditMode() { _isEditMode = true; }
	void DeActiveEditMode() { _isEditMode = false; }

	void SetOwnerWorld(class World* world) { _ownerWorld = world; }
	void SetActiveEidtMode(bool active) { _isEditMode = active; }
	bool GetIsEditMode() const { return _isEditMode; }

protected:
	class World* _ownerWorld;
	vector<UIBase*> _children;

	Vector2 _mousePos;

	bool _isEditMode = false;
};

