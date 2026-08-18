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

	virtual void Init(EAnchor anchor, EPivot pivot, Vector2 pos, Vector2 size, const std::wstring& text) {}
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

	// 이후 생성/Init되는 UIBase가 게임 뷰포트(GWinSizeX 기준) 대신 사이드 패널(GImGuiPanelWidth 기준)
	// 좌표계를 쓰게 한다. Widget_Main/Widget_Launch의 Init() 맨 앞뒤에서 켜고 끈다.
	static void SetActiveUISpace(bool usePanelSpace) { s_activeSpaceIsPanel = usePanelSpace; }

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

	// SetAnchor() 호출 시점의 s_activeSpaceIsPanel을 그대로 캡처해둔다 (Update()는 한참 뒤에도
	// 계속 불리니까, 그때도 자기가 어느 좌표계 소속인지 기억하고 있어야 함).
	bool _usePanelSpace = false;

private:
	static bool s_activeSpaceIsPanel;
};

