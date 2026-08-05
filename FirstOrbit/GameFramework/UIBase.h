#pragma once

class UIBase
{
public:
	UIBase() = default;
	virtual ~UIBase() = default;

	virtual void Init() {}
	virtual void Update(float deltaTime) {}
	virtual void Render(HDC hdc) {}
	virtual void OnGUI(){}

	void SetPos(float x, float y) { _x = x; _y = y; }
	void SetSize(float w, float h) { _width = w; _height = h; }
	void SetActive(bool active) { _isActive = active; }
	void SetColor(COLORREF color) { _color = color; }
	void SetTransparent(bool transparent) { _isTransparent = transparent; }


	bool IsActive() const { return _isActive; }

	string GetName() const { return _name; }

protected:
	float _x = 0.f, _y = 0.f;
	float _width = 0.f, _height = 0.f;
	bool _isActive = true;

	string _name = "";
	COLORREF _color = RGB(255, 255, 255);
	bool _isTransparent = false;
};

