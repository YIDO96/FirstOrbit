#pragma once

// WAV 파일 하나를 로드해서 재생하는 DirectSoundBuffer 래퍼.
class Sound
{
public:
	Sound() = default;
	~Sound();

	bool LoadWave(fs::path fullPath);
	void Play(bool loop = false);
	void Stop(bool reset = false);

private:
	LPDIRECTSOUNDBUFFER _soundBuffer = nullptr;
};
