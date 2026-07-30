#pragma once

struct FrameInfo
{
	int startX;
	int startY;
	int sizeX;
	int sizeY;
};

// Animation: 하나의 스프라이트시트에서 잘라낸 프레임 목록 + 재생 정보
// frameTime : 프레임 사이 간격
// isLoop : 반복 여부

class Texture;

class Animation
{
public:
	void Init(Texture* texture, Vector2 frameCount, FrameInfo info, float frameTime, float _deadTime, bool isloop = true);



	FrameInfo   GetFrame(int idx)   const { return _frames[idx]; }
	int32       GetFramesSize()     const { return _frames.size(); }
	Texture*    GetTextrue()        const { return _texture; }
	float       GetFrameTime()      const { return _frameTime; }
	bool        GetIsLoop()         const { return _isLoop; }

private:
	Texture*                _texture = nullptr;         // 스프라이트시트 이미지
	vector<FrameInfo>       _frames;                    // 애니메이션 프레임 배열
	float                   _frameTime = 0.1f;          // 애니메이션 재생 간격
	float                   _deadTime = 2.f;             // 애니메이션 전체 재생시간
	bool                    _isLoop = true;              // 반복여부
};
