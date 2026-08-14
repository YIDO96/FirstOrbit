#include "pch.h"
#include "Lambert.h"

// 람베르트
static float StumpffC(float z)
{
	if (z > 1e-6f) return (1.f - cosf(sqrtf(z))) / z;
	else if (z < -1e-6f) return (1.f - coshf(sqrtf(-z))) / z;
	else return 0.5f;
}

static float StumpffS(float z)
{
	// 타원 궤도 구간
	if (z > 1e-6f)
	{
		float sz = sqrtf(z);
		return (sz - sinf(sz)) / (sz * sz * sz);
	}
	// 쌍곡선 궤도 구간
	else if (z < -1e-6f)
	{
		float sz = sqrtf(-z);
		return (sinhf(sz) - sz) / (sz * sz * sz);
	}
	// 포물선 궤도 및 수치적 안정성 구간
	else return 1.f / 6.f;
}

LambertResult SolveLambert(Vector2 r1, Vector2 r2, float tof, float mu)
{
	LambertResult result;

	float r1n = r1.Length();
	float r2n = r2.Length();
	if (r1n < 0.01f or r2n < 0.001f or tof < 0.001f) return result;

	// 두 점 사이 각도 - 반시계 방향 기준으로 (0, 2pi) 보정
	float cosDNu = clamp(r1.Dot(r2) / (r1n * r2n), -1.f, 1.f);
	float dNu = acosf(cosDNu);
	if (r1.Cross(r2) < 0.f) dNu = 6.283185307f - dNu;

	float A = sinf(dNu) * sqrtf(r1n * r2n / (1.f - cosDNu));
	if (fabsf(A) < 1e-6f) return result; // 전이각 180도에 가까운 특이점 - 이번엔 실패처리

	auto ComputeTOF = [&](float z, float& outY) ->float
		{
			float C = StumpffC(z);
			float S = StumpffS(z);
			float y = r1n + r2n + A * (z * S - 1.f) / sqrtf(C);

			outY = y;
			if (y < 0.f) return -1.f;

			float chi = sqrtf(y / C);
			return (chi * chi * chi * S + A * sqrtf(y)) / sqrtf(mu);
		};

	// z ∈ [0, 4π²) 범위 = 포물선 경계 ~ 한 바퀴 도는 타원, 단일회전 해만 다룬다
	// (행성 간 전이는 항상 이 범위 안에서 답이 나옴 — 여러 바퀴 도는 해는 이번엔 스킵)

	// 추후 빠른 응급전이 같은 기능을 넣고 싶으면 zLow = -4.f * 3.14 * 3.14로 수정
	float zLow = 0.f;
	float zHigh = 4.f * 3.14159265f * 3.14159265f;
	float z = 0.f, y = 0.f;

	for (int i = 0; i < 100; ++i)
	{
		z = (zLow + zHigh) * 0.5f;
		float t = ComputeTOF(z, y);

		if (t < 0.f) { zHigh = z; continue; }
		if (t < tof) zLow = z;
		else zHigh = z;

		if (fabsf(t - tof) < 1e-3f) break;
	}

	if (y < 0.f) return result;

	float f = 1.f - y / r1n;
	float g = A * sqrtf(y / mu);
	float gDot = 1.f - y / r2n;
	if (fabsf(g) < 1e-9f) return result;

	result.v1 = (r2 - r1 * f) * (1.f / g);
	result.v2 = (r2 * gDot - r1) * (1.f / g);
	result.valid = true;



	return result;
}