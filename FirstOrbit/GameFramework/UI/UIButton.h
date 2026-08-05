#pragma once

#include "GameFramework/UIBase.h"

class UIButton : public UIBase
{
	using Super = UIBase;
public:
	using OnClickCallback = std::function<void()>;

public:

	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size) override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

	void SetOnClick(OnClickCallback callback) { _onClick = callback; }
	void SetTexture(class Texture* texture) { _texture = texture; }

	Texture* GetTexture() const { return _texture; }
private:
	OnClickCallback _onClick;

	class Texture* _texture = nullptr;
	Vector2 originSize = Vector2();
	//class UIText* _text = nullptr;
};

