#include "pch.h"
#include "ResourceData.h"

void ResourceData::Load(const json& data)
{
	// json 최상위 섹션(예: "LobbyScene", "EditorScene", "GameScene", "Sound")을 전부 순회한다.
	for (json::const_iterator sectionIt = data.begin(); sectionIt != data.end(); ++sectionIt)
	{
		wstring sectionName = Utf8ToWide(sectionIt.key());
		const json& sectionData = sectionIt.value();

		// "Sound"는 텍스처 섹션들과 스키마가 달라서 별도로 파싱한다.
		if (sectionName == L"Sound")
		{
			for (json::const_iterator it = sectionData.begin(); it != sectionData.end(); ++it)
			{
				auto key = it.key();
				auto value = it.value();

				SoundItem item;
				item.key = Utf8ToWide(key);
				item.fileName = Utf8ToWide(value["fileName"]);

				if (value.contains("loop"))
					item.loop = value["loop"];

				_soundSection.insert(make_pair(item.key, item));
			}
			continue;
		}

		Section section;

		for (json::const_iterator it = sectionData.begin(); it != sectionData.end(); ++it)
		{
			auto key = it.key(); // "BG", "Player.."
			auto value = it.value();	// { 데이터 }

			Item item;
			item.key = Utf8ToWide(key);
			item.fileName = Utf8ToWide(value["fileName"]);

			if (value.contains("transparent"))
			{
				item.transparent = RGB(value["transparent"][0], value["transparent"][1], value["transparent"][2]);
			}

			if (value.contains("countX"))
			{
				item.countX = value["countX"];
			}

			if (value.contains("countY"))
			{
				item.countY = value["countY"];
			}

			if (value.contains("loop"))
			{
				item.loop = value["loop"];
			}

			if (value.contains("dur"))
			{
				item.dur = value["dur"];
			}

			section.insert(make_pair(item.key, item));
		}

		_sections.insert(make_pair(sectionName, section));
	}
}

const ResourceData::Section* ResourceData::FindSection(const wstring& sectionName) const
{
	auto it = _sections.find(sectionName);
	if (it != _sections.end())
		return &it->second;

	return nullptr;
}
