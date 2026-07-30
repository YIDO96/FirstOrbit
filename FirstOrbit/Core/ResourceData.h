#pragma once

#include "DataObject.h"

// 텍스처/사운드 리소스 로딩 테이블 (Resources/Data/ResourceData.json)
// json 최상위 키(예: "LobbyScene", "EditorScene", "GameScene")별로 섹션을 나눠서 파싱한다.
// "Sound"는 텍스처 섹션들과 스키마가 달라서(fileName만 사용) 별도 SoundSection으로 분리해 담는다.
class ResourceData : public DataObject
{
public:
	struct Item
	{
		wstring key;
		wstring fileName;
		int32 transparent = -1;
		int32 countX = 1;
		int32 countY = 1;
		bool loop = false;
		float dur = 1.0f;
	};
	struct SoundItem
	{
		wstring key;
		wstring fileName;
		bool loop = false;
	};
	using Section = unordered_map<wstring, Item>;
	using SoundSection = unordered_map<wstring, SoundItem>;

public:
	virtual wstring GetFileName() override { return L"ResourceData.json"; }
	virtual void Load(const json& data) override;

	// sectionName(예: L"GameScene")에 해당하는 섹션을 찾는다. 없으면 nullptr.
	const Section* FindSection(const wstring& sectionName) const;
	const SoundSection& GetSoundSection() const { return _soundSection; }

public:
	map<wstring, Section> _sections;
	SoundSection _soundSection;
};
