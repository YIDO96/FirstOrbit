#include "pch.h"
#include "ResourceManager.h"
#include "GameFramework/Texture.h"
#include "Sound.h"

void ResourceManager::Init(HWND hwnd, fs::path directory)
{
	_resourcePath = directory;
}

void ResourceManager::LoadTexture(wstring key, wstring texturePath, int32 transparent, int32 row, int32 col, float dur)
{
	if (GetTexture(key) != nullptr)
	{
		// 이미 추가된 텍스처라서 무시
		return;
	}

	// 모든 텍스처(비트맵)는 Resources/Image/ 밑에 있다 (사운드는 Resources/Sound/).
	fs::path fullPath = _resourcePath / L"Image" / texturePath;

	// 텍스처 생성을 담당
	Texture* texture = new Texture();
	texture->Load(fullPath, transparent, row, col, dur);

	// 생성된 텍스터를 리소스 매니저가 관리
	_textures.insert(make_pair(key, texture));
}

Texture* ResourceManager::GetTexture(wstring key)
{
	auto find = _textures.find(key);
	if (find != _textures.end())
		return find->second;

	return nullptr;
}

void ResourceManager::LoadSound(wstring key, wstring soundFileName)
{
	if (GetSound(key) != nullptr)
	{
		// 이미 추가된 사운드라서 무시
		return;
	}

	// 모든 사운드는 Resources/Sound/ 밑에 있다 (텍스처는 Resources/Image/).
	fs::path fullPath = _resourcePath / L"Sound" / soundFileName;

	Sound* sound = new Sound();
	sound->LoadWave(fullPath);

	_sounds.insert(make_pair(key, sound));
}

Sound* ResourceManager::GetSound(wstring key)
{
	auto find = _sounds.find(key);
	if (find != _sounds.end())
		return find->second;

	return nullptr;
}
