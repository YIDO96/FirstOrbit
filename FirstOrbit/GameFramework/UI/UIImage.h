#pragma once

#include "GameFramework/UIBase.h"

class UIImage : public UIBase
{
	using Super = UIBase;

public:
	UIImage(class Texture* texture) : _texture(texture) {}

	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size) override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

private:
	class Texture* _texture = nullptr;
};

