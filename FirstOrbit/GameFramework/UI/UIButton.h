#pragma once

#include "GameFramework/UIBase.h"


class UIText;
class Texture;

class UIButton : public UIBase
{
	using Super = UIBase;
public:
	using OnClickCallback = std::function<void()>;

public:

	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text = L"") override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

	void SetOnClick(OnClickCallback callback) { _onClick = callback; }
	void SetTexture(class Texture* texture) { _texture = texture; }
	void SetText(EParentAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text = L"Text", float fontSize = 0.f);

	// SetActive(false)는 아예 안 보이게 숨기는 것. 이건 보이긴 하되(흐리게) 클릭/호버 반응은 죽인다 —
	// 우주선이 픽된 상태에서 "지금은 갈 수 없는 행성" 버튼을 표시할 때 씀.
	void SetInteractable(bool interactable) { _isInteractable = interactable; }
	bool IsInteractable() const { return _isInteractable; }

	Texture* GetTexture() const { return _texture; }
	UIText* GetText() const { return _text; }
private:
	OnClickCallback _onClick;
	bool _isInteractable = true;

	Texture* _texture = nullptr;
	UIText* _text = nullptr;
	Vector2 _originSize = Vector2();
	float _originFontSize = 0.f;
	float _hoverRate = 1.1f;

	EParentAnchor _textAnchor = EParentAnchor::Center;
	EPivot        _textPivot = EPivot::Center;
	Vector2       _textPos = Vector2();
	Vector2       _textSize = Vector2();
};

