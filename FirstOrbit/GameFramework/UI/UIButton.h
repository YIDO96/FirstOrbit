#pragma once

#include "GameFramework/UIBase.h"

class UIButton : public UIBase
{
	using Super = UIBase;
public:
	using OnClickCallback = std::function<void()>;

public:

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

	void SetOnClick(OnClickCallback callback) { _onClick = callback; }


private:
	OnClickCallback _onClick;
};

