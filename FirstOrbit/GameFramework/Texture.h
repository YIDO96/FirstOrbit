#pragma once

class Texture
{
public:
	~Texture();

	void Load(wstring texturePath, int32 transparent);
	void Load(wstring texturePath, int32 transparent, int32 row, int32 col, float dur);
	void Render(HDC hdc, Vector2 renderPos, Vector2 srcPos = Vector2(), 
			Vector2 srcSize = Vector2(), Vector2 destSize = Vector2());

	// 회전 각도(라디안)를 받아 회전 렌더링을 수행
	void RenderRotated(HDC hdc, Vector2 centerPos, float radian, Vector2 destSize = Vector2());

	Vector2 GetTextureSize() const { return Vector2(_bitmapSizeX, _bitmapSizeY); }

private:
	HDC			_bitmapHdc = 0;
	HBITMAP		_bitmap = 0;
	// 투명 마스크 처리를 위한 마스크 비트맵 정보
	HBITMAP     _maskBitmap = 0;

	HDC			_tempDC = nullptr;
	HBITMAP		_tempBitmap = nullptr;
	int32		_tempDim = 0;

	int32		_transparent = -1;
	uint32		_bitmapSizeX = 0;
	uint32		_bitmapSizeY = 0;


	// 행/열로 쪼개진 텍스처의 경우, 행/열 개수를 저장한다.
	int32		_col = 0;
	int32		_row = 0;
	int32		_frameSizeX = 0;
	int32		_frameSizeY = 0;
	float		_dur = 0;
};
