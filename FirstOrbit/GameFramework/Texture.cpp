#include "pch.h"
#include "Texture.h"
#include "Core/GameInstance.h"
#include "GameFramework/Camera.h"



Texture::~Texture()
{
	// DC를 먼저 삭제하면 여기 선택되어 있던 비트맵이 자동으로 해제(deselect)된다.
	if (_bitmapHdc)
		::DeleteDC(_bitmapHdc);
	if (_bitmap)
		::DeleteObject(_bitmap);
	if (_maskBitmap)
		::DeleteObject(_maskBitmap);
}

void Texture::Load(wstring texturePath, int32 transparent)
{
	HDC hdc = ::GetDC(GameInstance::GetInstance().GetHwnd());

	_bitmapHdc = ::CreateCompatibleDC(hdc);
	_bitmap = (HBITMAP)::LoadImageW(
		nullptr,
		texturePath.c_str(),
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION
	);

	// 투명하게 보여야하는 색상값
	_transparent = transparent;

	if (_bitmap == 0)
	{
		::MessageBox(GameInstance::GetInstance().GetHwnd(), texturePath.c_str(), L"Invalid Texture Load", MB_OK);
		return;
	}

	HBITMAP prev = (HBITMAP)::SelectObject(_bitmapHdc, _bitmap);
	::DeleteObject(prev);

	BITMAP bit = {};
	::GetObject(_bitmap, sizeof(BITMAP), &bit);

	_bitmapSizeX = bit.bmWidth;
	_bitmapSizeY = bit.bmHeight;

	::ReleaseDC(GameInstance::GetInstance().GetHwnd(), hdc);
}

void Texture::Load(wstring texturePath, int32 transparent, int32 row, int32 col, float dur)
{
	HDC hdc = ::GetDC(GameInstance::GetInstance().GetHwnd());

	_bitmapHdc = ::CreateCompatibleDC(hdc);
	_bitmap = (HBITMAP)::LoadImageW(
		nullptr,
		texturePath.c_str(),
		IMAGE_BITMAP,
		0,
		0,
		LR_LOADFROMFILE | LR_CREATEDIBSECTION
	);

	// 투명하게 보여야하는 색상값
	_transparent = transparent;

	if (_bitmap == 0)
	{
		::MessageBox(GameInstance::GetInstance().GetHwnd(), texturePath.c_str(), L"Invalid Texture Load", MB_OK);
		return;
	}

	HBITMAP prev = (HBITMAP)::SelectObject(_bitmapHdc, _bitmap);
	::DeleteObject(prev);

	BITMAP bit = {};
	::GetObject(_bitmap, sizeof(BITMAP), &bit);

	_bitmapSizeX = bit.bmWidth;
	_bitmapSizeY = bit.bmHeight;

	// 행/열로 쪼개진 sprite
	_col = col;
	_row = row;
	_dur = dur;

	// 쪼개진 1개의 frame size
	_frameSizeX = _bitmapSizeX / _col;
	_frameSizeY = _bitmapSizeY / _row;
}

void Texture::Render(HDC hdc, Vector2 renderPos, Vector2 srcPos, Vector2 srcSize, Vector2 destSize)
{
	if (srcSize == Vector2()) srcSize = GetTextureSize();
	if (destSize == Vector2()) destSize = srcSize;


	if (_transparent == -1)
	{
		::SetStretchBltMode(hdc, COLORONCOLOR);
		::StretchBlt(
			hdc,
			(int32)renderPos.x,
			(int32)renderPos.y,
			(int32)(destSize.x),  // [수정] 화면에 그려질 최종 너비 (줌 적용)
			(int32)(destSize.y),  // [수정] 화면에 그려질 최종 높이 (줌 적용)
			_bitmapHdc,
			(int32)srcPos.x,
			(int32)srcPos.y,
			srcSize.x,                  // [추가] 원본 비트맵의 너비
			srcSize.y,                  // [추가] 원본 비트맵의 높이
			SRCCOPY
		);
	}
	else
	{
		::TransparentBlt(hdc,
			(int32)renderPos.x,								// 윈도우 좌표 어디에 그릴지
			(int32)renderPos.y,								// 윈도우 좌표 어디에 그릴지
			(int32)(destSize.x),	// 윈도우 좌표에 그려질 최종 크기
			(int32)(destSize.y),	// 윈도우 좌표에 그려질 최종 크기
			_bitmapHdc,										// 해당 비트맵을 그려줘
			(int32)srcPos.x,								// 그리고 싶은 비트맵의 좌표
			(int32)srcPos.y,								// 그리고 싶은 비트맵의 좌표
			srcSize.x,											// 그리고 싶은 비트맵의 크기
			srcSize.y,											// 그리고 싶은 비트맵의 크기
			_transparent);									// 투명 키값(RGB)
	}
}

void Texture::RenderRotated(HDC hdc, Vector2 centerPos, float radian)
{
	// 회전 전 로컬 기준의 3개 꼭짓점 계산 좌상, 우상, 좌하 순
	float halfX = _bitmapSizeX / 2.f;
	float halfY = _bitmapSizeY / 2.f;

	Vector2 localCorners[3] = {
		{ -halfX, -halfY }, // 좌상
		{  halfX, -halfY }, // 우상
		{ -halfX,  halfY }  // 좌하
	};

	// 삼각함수를 이용하여 3개 꼭짓점을 중심점(centerPos) 기준으로 월드 회전 변환
	POINT destPoints[3];

	for (int i = 0; i < 3; ++i)
	{
		Vector2 rotatePos = localCorners[i].Rotate(radian);

		destPoints[i].x = static_cast<long>(rotatePos.x + centerPos.x);
		destPoints[i].y = static_cast<long>(rotatePos.y + centerPos.y);
	}

	// _maskBitmap 없이 투명키 배경을 지우며 회전한다.
	if (_transparent != -1)
	{
		// 회전된 이미지가 안전하게 다 들어갈 수 있도록 넉넉한 크기의 임시 버퍼 생성
		int32 tempSizeX = (int32)(_bitmapSizeX * 1.5f);
		int32 tempSizeY = (int32)(_bitmapSizeY * 1.5f);

		HDC tempDC = ::CreateCompatibleDC(hdc);
		HBITMAP tempBitmap = ::CreateCompatibleBitmap(hdc, tempSizeX, tempSizeY);
		HBITMAP oldTemp = (HBITMAP)::SelectObject(tempDC, tempBitmap);

		// 임시 버퍼의 배경을 투명 키값으로 통일해서 채워둔다.
		HBRUSH bgBrush = ::CreateSolidBrush(_transparent);
		RECT r = { 0, 0, tempSizeX,tempSizeY };
		::FillRect(tempDC, &r, bgBrush);
		::DeleteObject(bgBrush);

		// 임시 버퍼 내부의 정중앙에 회전된 꼭짓점이 맺히도록 오프셋 좌표 보정
		POINT rotatedDestPoints[3];
		float offsetX = tempSizeX / 2.f;
		float offsetY = tempSizeY / 2.f;

		for (int i = 0; i < 3; ++i)
		{
			Vector2 rotatePos = localCorners[i].Rotate(radian);
			rotatedDestPoints[i].x = static_cast<long>(rotatePos.x + offsetX);
			rotatedDestPoints[i].y = static_cast<long>(rotatePos.y + offsetY);
		}

		// TransparentBlt로 임시 버퍼에 원본을 회전 없이 정방향으로 복사한다.
		::PlgBlt(tempDC, rotatedDestPoints, _bitmapHdc, 0, 0, _bitmapSizeX, _bitmapSizeY, NULL, 0, 0);

		// 배경 처리가 끝난 tempDC 자체를 PlgBlt로 회전 출력한다.
		int32 drawX = (int32)(centerPos.x - offsetX);
		int32 drawY = (int32)(centerPos.y - offsetY);

		::TransparentBlt(hdc, drawX, drawY, tempSizeX, tempSizeY,
			tempDC, 0, 0, tempSizeX, tempSizeY, _transparent);

		::SelectObject(tempDC, oldTemp);
		::DeleteObject(tempBitmap);
		::DeleteDC(tempDC);
	}
	else
	{
		// 투명 키값이 없는 이미지라면 마스크 없이 원본 그대로 바로 PlgBlt 회전 수행
		::PlgBlt(hdc, destPoints, _bitmapHdc, 0, 0, _bitmapSizeX, _bitmapSizeY, NULL, 0, 0);
	}
}
