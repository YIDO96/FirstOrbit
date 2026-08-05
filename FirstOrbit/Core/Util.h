#pragma once

float RadianToDegree(float radian);
float DegreeToRadian(float degree);

float LerpAngle(float from, float to, float t);


std::wstring CharToWStringStandard(const char* str);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);