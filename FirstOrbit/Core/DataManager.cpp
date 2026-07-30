#include "pch.h"
#include "DataManager.h"
#include "ResourceData.h"

void DataManager::Init(fs::path directory)
{
	_resourcePath = directory;
}

void DataManager::Load()
{
	fs::path directory = _resourcePath / L"Data/";

	{
		ResourceData* data = new ResourceData();
		loadDataObject(directory, L"ResourceData", data);
	}
}

void DataManager::loadDataObject(fs::path directory, wstring key, DataObject* obj)
{
	if (_datas.find(key) != _datas.end())
	{
		// 중복 로드 요청, 이미 존재하는 키라서 무시
		return;
	}

	fs::path path = directory / obj->GetFileName();
	std::ifstream file(path.c_str());
	if (!file.is_open())
	{
		MessageBox(nullptr, L"Failed to open JSON file", L"Error", MB_OK);
		return;
	}

	json data = json::parse(file);
	obj->Load(data); // 읽은 json 데이터를 형식에 맞춰서 C++ 객체로 변환

	_datas.emplace(key, obj);
}
