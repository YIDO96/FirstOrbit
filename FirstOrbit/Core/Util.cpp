#include "pch.h"
#include "Util.h"
#include <numbers>  // C++20


float RadianToDegree(float radian)
{
	float degree = radian * (180.0f / std::numbers::pi_v<float>);

	degree = fmodf(degree, 360.0f);
	if (degree < 0.0f)
		degree += 360.0f;

	return degree;
}

float DegreeToRadian(float degree)
{
	float radian = degree * (std::numbers::pi_v<float> / 180.0f);

	radian = fmodf(radian, std::numbers::pi_v<float> *2);
	if (radian < 0.0f)
		radian += std::numbers::pi_v<float> *2;

	return radian;
}

float LerpAngle(float from, float to, float t)
{
	float delta = to - from;
	delta = atan2f(sinf(delta), cosf(delta));   // ±π 안으로 접기
	return from + delta * t;
}

std::wstring CharToWStringStandard(const char* str)
{
	// C locale 설정 (한글 처리를 위해 필수)
	std::setlocale(LC_ALL, "");

	size_t len = strlen(str) + 1;
	std::wstring wstr(len, L'\0');

	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, &wstr[0], len, str, _TRUNCATE);

	if (!wstr.empty() && wstr.back() == L'\0') {
		wstr.pop_back();
	}

	return wstr;
}

std::wstring StringToWString(const std::string& str)
{
	if (str.empty()) return L"";

	// ImGui 등 UTF-8 기반이면 CP_UTF8, 일반 멀티바이트(한글 윈도우)면 CP_ACP 사용
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], sizeNeeded);

	return wstrTo;
}

// std::wstring -> std::string 변환
std::string WStringToString(const std::wstring& wstr)
{
	if (wstr.empty()) return "";

	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);

	return strTo;
}
