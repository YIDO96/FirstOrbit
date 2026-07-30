#pragma once

#include "UActorComponent.h"

// UAnimatorComponent: 액터에 붙어 스프라이트시트 애니메이션을 재생하는 컴포넌트
// map<string, Animation*> animList  애니메이션 목록
// Animation* currentAnim            현재 재생 애니메이션
// int currentFrameIndex             보여주고 있는 FrameIndex
// float allTime                     프레임 전환을 위한 누적 시간

class Animation;

class UAnimatorComponent : public UActorComponent
{
public:
	UAnimatorComponent(AActor* owner) : UActorComponent(owner) {}
	virtual ~UAnimatorComponent();

	virtual void Update(float deltaTime) override { UpdateAnimation(deltaTime); }
	virtual void Render(HDC hdc) override { RenderAnimation(hdc); }

	void UpdateAnimation(float deltaTime);
	void RenderAnimation(HDC hdc);

	void PlayAnimation(string key, Vector2 pos);
	void AddAnimation(wstring str, Vector2 frameCount, float frameTime = 0.1f, float deadTime = 2.f, bool isLoop = true);

	void SetPosition(Vector2 pos);

	// isLoop == false인 애니메이션이 마지막 프레임까지 다 재생됐는지 여부
	bool IsFinished() const { return _isFinished; }


private:
	map<string, Animation*> _animList;
	Animation*				_currentAnim;
	Vector2					_position;

	int						_currentFrameIndex;
	float					_accTime;
	bool					_isFinished = false;
};
