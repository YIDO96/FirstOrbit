#pragma once

#include "Singleton.h"

// DirectSound 디바이스를 소유하고, ResourceManager에 로드된 사운드를 키로 재생한다.
class SoundManager : public Singleton<SoundManager>
{
	// Singleton 객체를 '친구'로 선언해서 private 접근 가능하게 열어준다.
	friend Singleton<SoundManager>;

public:
	void Init(HWND hwnd);
	void Play(const wstring& key, bool loop = false);

	LPDIRECTSOUND GetSoundDevice() { return _soundDevice; }

private:
	// 아무나 생성못하게 생성자/소멸자를 숨기자
	SoundManager() = default;
	~SoundManager();

private:
	LPDIRECTSOUND _soundDevice = nullptr;
};

#define SOUND SoundManager::GetInstance()
