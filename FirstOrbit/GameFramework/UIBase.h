#pragma once

enum class EAnchor
{
	LeftTop,
	Top,
	RightTop,

	Left,
	Center,
	Right,

	LeftBottom,
	Bottom,
	RightBottom
};

enum class EParentAnchor
{
	LeftTop,
	Top,
	RightTop,

	Left,
	Center,
	Right,

	LeftBottom,
	Bottom,
	RightBottom
};

enum class EPivot
{
	LeftTop,
	Top,
	RightTop,
	Left,
	Center,
	Right,
	LeftBottom,
	Bottom,
	RightBottom
};

class UIBase
{
public:
	UIBase() = default;
	virtual ~UIBase() = default;

	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size) {}
	virtual void Update(float deltaTime) {}
	virtual void Render(HDC hdc){}
	virtual void OnGUI(){}

	void RecalculateFinalPos();

	bool IsHoverInUI(Vector2 mousePos);

	void SetParentUI(UIBase* parent) { _parent = parent; }
	void SetPos(float x, float y);
	virtual void SetSize(float w, float h);
	void SetActive(bool active) { _isActive = active; }
	void SetColor(COLORREF color) { _color = color; }
	void SetTransparent(bool transparent) { _isTransparent = transparent; }

	void SetAnchor(EAnchor anchor);
	void SetParentAnchor(EParentAnchor parentanchor, UIBase* parent);
	void SetPivot(EPivot pivot);

	Vector2 GetPos() { return Vector2(_finalX, _finalY); }
	Vector2 GetSize() { return Vector2(_width, _height); }
	bool IsActive() const { return _isActive; }
	bool IsTransparent() const { return _isTransparent; }
	COLORREF GetColor() const { return _color; }
	string GetName() const { return _name; }

protected:
	UIBase* _parent = nullptr;

	float _x = 0.f, _y = 0.f;
	float _anchorX = 0.f, _anchorY = 0.f;
	float _pivotRatioX = 0.f, _pivotRatioY = 0.f;
	float _finalX = 0.f, _finalY = 0.f;
	float _width = 0.f, _height = 0.f;
	bool _isActive = true;

	EAnchor _anchor = EAnchor::LeftTop;
	EParentAnchor _parentAnchor = EParentAnchor::Center;
	EPivot _pivot = EPivot::LeftTop;

	string _name = "";
	COLORREF _color = RGB(255, 255, 255);
	bool _isTransparent = false;

};

