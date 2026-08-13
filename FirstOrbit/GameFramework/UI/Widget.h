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

	// 자식 UI 요소(버튼 등) 중 하나라도 마우스가 위에 있으면 true.
	// (Widget 자기 자신은 크기를 안 갖고 있어서 IsHoverInUI를 그대로 쓰면 항상 false가 나온다)
	bool IsMouseOverUI(Vector2 mousePos) const;

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

