#pragma once

class DataObject
{
public:
	// 파생 클래스가 fileName을 반환하고, 로딩 함수를 구현한다.
	virtual wstring GetFileName() = 0;
	virtual void Load(const json& data) = 0;
};

std::string Utf8ToAnsi(const std::string& utf8Str);
std::wstring Utf8ToWide(const std::string& utf8Str);
