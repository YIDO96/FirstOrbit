#pragma once


class Glow
{
public:
	Glow() = default;
	~Glow();

	bool Create(int size, COLORREF color);
	void Render(HDC hdcDest, const Vector2& centerPos, float radius, BYTE intensity = 255);
	void Destroy();

private:
	HBITMAP _hBitmap = NULL;
	HDC _hdc = NULL;
	int _size = 0;

	float _k = 8.f;
};