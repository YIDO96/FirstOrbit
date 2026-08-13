#pragma once

float RadianToDegree(float radian);
float DegreeToRadian(float degree);

float LerpAngle(float from, float to, float t);

float Normalize(float value, float min, float max);
COLORREF LerpColor(const COLORREF& colorA, const COLORREF& colorB, float t);

float SolveKeplerEquation(float meanAnomaly, float eccentricity);

std::wstring CharToWStringStandard(const char* str);
std::wstring StringToWString(const std::string& str);
std::string WStringToString(const std::wstring& wstr);