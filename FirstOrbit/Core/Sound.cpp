#include "pch.h"
#include "Sound.h"
#include "SoundManager.h"

Sound::~Sound()
{
	if (_soundBuffer)
		_soundBuffer->Release();
}

bool Sound::LoadWave(fs::path fullPath)
{
	if (fullPath.extension() != L".wav")
		return false;

	// 오디오 디바이스가 없으면(생성 실패) 사운드는 조용히 스킵한다.
	// (GetSoundDevice()가 null이면 아래 CreateSoundBuffer에서 null 역참조로 크래시하므로 여기서 차단)
	if (SoundManager::GetInstance().GetSoundDevice() == nullptr)
		return false;

	HMMIO file = ::mmioOpen((WCHAR*)fullPath.c_str(), NULL, MMIO_READ);
	if (file == 0)
	{
		::OutputDebugStringW(L"[Sound] 사운드 파일 열기 실패\n");
		return false;
	}

	MMCKINFO parent = {};
	parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	::mmioDescend(file, &parent, NULL, MMIO_FINDRIFF);

	MMCKINFO child = {};
	child.ckid = mmioFOURCC('f', 'm', 't', ' ');
	::mmioDescend(file, &child, &parent, MMIO_FINDCHUNK);

	WAVEFORMATEX wfx = {};
	::mmioRead(file, (char*)&wfx, sizeof(wfx));

	// PCM의 fmt 청크는 16바이트인데 sizeof(WAVEFORMATEX)=18로 읽어 2바이트를 넘겨 읽는다.
	// 그 2바이트(뒤 data 청크의 'd''a')가 cbSize에 쓰레기값으로 들어가는데,
	// PCM은 cbSize가 0이어야 하며 엄격한 오디오 드라이버는 비-0 cbSize면 CreateSoundBuffer를 거부한다.
	wfx.cbSize = 0;

	::mmioAscend(file, &child, 0);
	child.ckid = mmioFOURCC('d', 'a', 't', 'a');
	::mmioDescend(file, &child, &parent, MMIO_FINDCHUNK);

	DSBUFFERDESC bufferDesc = {};
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_STATIC;
	bufferDesc.dwBufferBytes = child.cksize;
	bufferDesc.lpwfxFormat = &wfx;

	if (FAILED(SoundManager::GetInstance().GetSoundDevice()->CreateSoundBuffer(&bufferDesc, &_soundBuffer, NULL)))
	{
		::OutputDebugStringW(L"[Sound] 사운드 버퍼 생성 실패 - 이 사운드는 무음 처리\n");
		::mmioClose(file, 0);
		return false;
	}

	void* write1 = nullptr;
	void* write2 = nullptr;
	DWORD len1 = 0;
	DWORD len2 = 0;

	_soundBuffer->Lock(0, child.cksize, &write1, &len1, &write2, &len2, 0);

	if (write1)
		::mmioRead(file, (char*)write1, len1);
	if (write2)
		::mmioRead(file, (char*)write2, len2);

	_soundBuffer->Unlock(write1, len1, write2, len2);

	::mmioClose(file, 0);
	return true;
}

void Sound::Play(bool loop)
{
	if (!_soundBuffer) return;

	_soundBuffer->SetCurrentPosition(0);
	_soundBuffer->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);
}

void Sound::Stop(bool reset)
{
	if (!_soundBuffer) return;

	_soundBuffer->Stop();
	if (reset)
		_soundBuffer->SetCurrentPosition(0);
}
