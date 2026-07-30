#pragma once

#include "Singleton.h"

class DataObject;

class DataManager : public Singleton<DataManager>
{
	// Singleton 객체를 '친구'로 선언해서 private 접근 가능하게 열어준다.
	friend Singleton<DataManager>;

public:
	void Init(fs::path directory);
	void Load();

	template<typename T>
	T* FindData(wstring key)
	{
		auto find = _datas.find(key);
		if (find != _datas.end())
		{
			return dynamic_cast<T*>(find->second);
		}

		return nullptr;
	}

private:
	void loadDataObject(fs::path directory, wstring key, DataObject* obj);

private:
	// 모든 데이터 json 파일이 이 루트 디렉터리 아래 있다고 가정한다.
	fs::path _resourcePath;

	// key, data
	map<wstring, class DataObject*>	 _datas;
};

#define DATA DataManager::GetInstance()
