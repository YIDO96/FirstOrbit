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

float Normalize(float value, float min, float max)
{
	if (abs(max - min) < 1e-6f) return 0.f;

	float normalized = (value - min) / (max - min);

	return clamp(normalized, 0.0f, 1.0f);
}

COLORREF LerpColor(const COLORREF& colorA, const COLORREF& colorB, float t)
{

	BYTE rA = GetRValue(colorA);
	BYTE gA = GetGValue(colorA);
	BYTE bA = GetBValue(colorA);

	BYTE rB = GetRValue(colorB);
	BYTE gB = GetGValue(colorB);
	BYTE bB = GetBValue(colorB);

	BYTE r = static_cast<BYTE>(rA + (rB - rA) * t + 0.5f);
	BYTE g = static_cast<BYTE>(gA + (gB - gA) * t + 0.5f);
	BYTE b = static_cast<BYTE>(bA + (bB - bA) * t + 0.5f);

	return RGB(r, g, b);
}

float SolveKeplerEquation(float meanAnomaly, float eccentricity)
{
	// 뉴턴법 : E = M + e*sin(E)를 반복 수렴
	// 이심률(최대 수성 0.206)범위에서는 6회면 충분히 수렴 가능
	float E = meanAnomaly;
	for (int i = 0; i < 6; i++)
	{
		float dE = (E - eccentricity * sinf(E) - meanAnomaly) / (1.f - eccentricity * cosf(E));
		E -= dE;
	}

	return E;
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
