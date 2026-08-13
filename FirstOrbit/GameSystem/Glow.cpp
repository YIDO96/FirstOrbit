#include "pch.h"
#include "Glow.h"

Glow::~Glow()
{
	Destroy();
}

bool Glow::Create(int size, COLORREF color)
{
	Destroy();
	_size = size;

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = size;
	bmi.bmiHeader.biHeight = -size; // Top-Down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	BYTE* pPixels = nullptr;
	HDC hScreenDC = GetDC(NULL);
	_hBitmap = CreateDIBSection(hScreenDC, &bmi, DIB_RGB_COLORS, (void**)&pPixels, NULL, 0);
	ReleaseDC(NULL, hScreenDC);

	if (!_hBitmap) return false;

	_hdc = CreateCompatibleDC(NULL);
	SelectObject(_hdc, _hBitmap);

	BYTE r = GetRValue(color);
	BYTE g = GetGValue(color);
	BYTE b = GetBValue(color);
	float center = size / 2.f;

	for (int y = 0; y < size; ++y)
	{
		for (int x = 0; x < size; ++x)
		{
			float dx = x - center;
			float dy = y - center;
			float distNorm = std::sqrt(dx * dx + dy * dy) / center; // dist/Radius

			// 1. 방사형 광원 감쇠식 (Inverse Square 법칙 기반)
			float factor = 1.f / (1.f + _k * (distNorm * distNorm));
			factor = max(0.f, factor - 0.2f) / 0.8f; // 외곽 테두리 정리

			BYTE a = (BYTE)(clamp(factor, 0.f, 1.f) * 255.f);

			// Permultiplied Alpha 포맷 변환
			int idx = (y * size + x) * 4;
			pPixels[idx]	 = (BYTE)((b * a) / 255);	// B
			pPixels[idx + 1] = (BYTE)((g * a) / 255);	// G
			pPixels[idx + 2] = (BYTE)((r * a) / 255);	// R
			pPixels[idx + 3] = a;						// A
		}
	}

	return true;
}

bool Glow::Render(HDC hdcDest, const Vector2& centerPos, float radius, BYTE intensity)
{
	if (not _hdc) return true;

	int destSize = (int)(radius * 2.f);
	int destX = (int)(centerPos.x - radius);
	int destY = (int)(centerPos.y - radius);

	BLENDFUNCTION bf = { 0 };
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = intensity;
	bf.AlphaFormat = AC_SRC_ALPHA;

	//AlphaBlend(hdcDest, destX, destY, destSize, destSize, _hdc, 0, 0, _size, _size, bf);
	return AlphaBlend(hdcDest, destX, destY, destSize, destSize, _hdc, 0, 0, _size, _size, bf) != FALSE;
}

void Glow::Destroy()
{
	if (_hdc) { DeleteDC(_hdc); _hdc = NULL; }
	if (_hBitmap) { DeleteObject(_hBitmap); _hBitmap = NULL; }
}
