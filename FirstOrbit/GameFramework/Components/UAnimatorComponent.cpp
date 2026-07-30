#include "pch.h"
#include "UAnimatorComponent.h"

#include "Core/ResourceManager.h"
#include "GameFramework/Texture.h"
#include "GameFramework/Animation.h"

UAnimatorComponent::~UAnimatorComponent()
{
	// _currentAnim은 _animList 안의 항목을 가리키는 비소유 포인터라 따로 delete하지 않는다.
	for (auto& pair : _animList)
		delete pair.second;
}

void UAnimatorComponent::UpdateAnimation(float deltaTime)
{
	if (_currentAnim == nullptr || _isFinished) return;

	_accTime += deltaTime;
	if (_accTime >= _currentAnim->GetFrameTime())
	{
		_accTime -= _currentAnim->GetFrameTime();
		_currentFrameIndex++;

		if (_currentFrameIndex >= _currentAnim->GetFramesSize())
		{
			if (_currentAnim->GetIsLoop())
			{
				_currentFrameIndex = 0;
			}
			else
			{
				_currentFrameIndex = _currentAnim->GetFramesSize() - 1; // 고정
				_isFinished = true;
			}
		}
	}
}

void UAnimatorComponent::RenderAnimation(HDC hdc)
{
	if (_currentAnim == nullptr || _isFinished) return;

	FrameInfo frame = _currentAnim->GetFrame(_currentFrameIndex);

	_currentAnim->GetTextrue()->Render(hdc, _position, Vector2(frame.startX, frame.startY), Vector2(frame.sizeX, frame.sizeY));
}

void UAnimatorComponent::PlayAnimation(string key, Vector2 pos) // pos는 센터값으로
{
	// 같은 애니메이션이 "재생 중"이면 무시. 단, 이미 끝난 원샷 애니메이션은
	// 같은 키라도 처음부터 다시 재생해야 하므로 _isFinished일 땐 통과시킨다.
	if (_currentAnim == _animList[key] && !_isFinished) return;

	_currentAnim = _animList[key];
	_currentFrameIndex = 0;
	_accTime = 0;
	_isFinished = false;

	SetPosition(pos);
}

void UAnimatorComponent::SetPosition(Vector2 pos)
{
	if (_currentAnim == nullptr) return;

	// pos를 애니메이션의 "중심 좌표"로 받아서, 실제 Blt에 쓰일 좌상단 좌표로 변환해 저장한다.
	// (모든 프레임의 sizeX/sizeY는 동일하므로 0번 프레임 기준으로 계산해도 무방)
	FrameInfo frame = _currentAnim->GetFrame(0);
	_position = Vector2(pos.x - frame.sizeX / 2.f, pos.y - frame.sizeY / 2.f);
}

void UAnimatorComponent::AddAnimation(wstring keyStr, Vector2 frameCount, float frameTime, float deadTime, bool isLoop)
{
	// 텍스처는 ResourceData.json을 통해 DataManager가 이미 로드해둔 것을 가져다 쓴다.
	Texture* texture = ResourceManager::GetInstance().GetTexture(keyStr);
	FrameInfo info;
	if (frameCount.y == 0)
	{
		info = FrameInfo(0, 0, texture->GetTextureSize().x / frameCount.x, texture->GetTextureSize().y);
	}
	else
	{
		info = FrameInfo(0, 0, texture->GetTextureSize().x / frameCount.x, texture->GetTextureSize().y / frameCount.y);
	}



	Animation* anim = new Animation();
	anim->Init(texture, frameCount, info, frameTime, deadTime, isLoop);
	string key = string().assign(keyStr.begin(), keyStr.end());


	_animList[key] = anim;
}
