#include "pch.h"
#include "Animation.h"

void Animation::Init(Texture* texture, Vector2 frameCount, FrameInfo info, float frameTime, float deadTime, bool isloop)
{
	_texture = texture;
	_frameTime = frameTime;
	_deadTime = deadTime;
	_isLoop = isloop;

	// 이미지 스프라이트를 프레임 단위로 잘라낸다
	if (frameCount.y == 0)
	{
		for (int x = 0; x < frameCount.x; ++x)
		{
			_frames.push_back(FrameInfo(info.startX + (x * info.sizeX),
				info.startY,
				info.sizeX,
				info.sizeY));
		}
	}
	else
	{
		// 세로
		for (int y = 0; y < frameCount.y; ++y)
		{
			// 가로
			for (int x = 0; x < frameCount.x; ++x)
			{
				_frames.push_back(FrameInfo(info.startX + (x * info.sizeX),
					info.startY + (y * info.sizeY),
					info.sizeX,
					info.sizeY));
			}
		}
	}

	_isLoop = isloop;
}
