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
