#include "pch.h"
#include "SoundManager.h"
#include "ResourceManager.h"
#include "Sound.h"

SoundManager::~SoundManager()
{
	if (_soundDevice)
		_soundDevice->Release();
}

void SoundManager::Init(HWND hwnd)
{
	// 사운드는 게임의 필수 요소가 아니다. 오디오 장치가 없거나(기본 재생 장치 부재 등)
	// 초기화에 실패해도 게임은 돌아가야 하므로, 모달 MessageBox로 막지 않고
	// 디바이스 없이 "무음"으로 조용히 진행한다 (fail-soft).

	// 사운드 디바이스 생성
	if (FAILED(::DirectSoundCreate(NULL, &_soundDevice, NULL)))
	{
		::OutputDebugStringW(L"[Sound] DirectSound 디바이스 생성 실패 - 무음으로 진행\n");
		_soundDevice = nullptr;
		return;
	}

	// 사운드 디바이스 협조 수준 설정
	if (FAILED(_soundDevice->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)))
	{
		::OutputDebugStringW(L"[Sound] 협조 수준 설정 실패 - 무음으로 진행\n");
		_soundDevice->Release();
		_soundDevice = nullptr;
		return;
	}
}

void SoundManager::Play(const wstring& key, bool loop /*= false*/)
{
	Sound* sound = ResourceManager::GetInstance().GetSound(key);
	if (sound == nullptr)
		return;

	sound->Play(loop);
}

void SoundManager::Stop(const wstring& key, bool reset)
{
	Sound* sound = ResourceManager::GetInstance().GetSound(key);
	if (sound == nullptr)
		return;

	sound->Stop(reset);
}

void SoundManager::SetVolume(const wstring& key, float percent)
{
	Sound* sound = ResourceManager::GetInstance().GetSound(key);
	if (sound == nullptr)
		return;

	sound->SetVolumePercent(percent);
}
