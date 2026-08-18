#pragma once

struct LambertResult
{
	Vector2 v1;      // 출발 시 필요한 속도
	Vector2 v2;      // 도착 시 속도
	float transferAngle = 0.f;
	bool valid = false;
};

LambertResult SolveLambert(Vector2 r1, Vector2 r2, float tof, float mu);