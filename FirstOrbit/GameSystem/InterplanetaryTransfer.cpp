#include "pch.h"
#include "InterplanetaryTransfer.h"

#include "Actor/APlanet.h"
#include "Lambert.h"

static float ComputeHohmannTOF(float r1, float r2, float mu)
{
	float a_t = (r1 + r2) * 0.5f;
	return 3.14159265f * sqrtf(a_t * a_t * a_t / mu);
}

float ComputePhaseAngleError(Vector2 shipPos, APlanet* target, Vector2 sunPos, float sunMu)
{
	Vector2 r1v = shipPos - sunPos;
	Vector2 r2v = target->GetCenterPos() - sunPos;
	float r1 = r1v.Length();
	float r2 = r2v.Length();

	float hohmannTOF = ComputeHohmannTOF(r1, r2, sunMu);
	float idealLead = 3.14159265f - target->GetOrbitSpeed() * hohmannTOF;
	idealLead = fmodf(idealLead, 6.283185307f);
	if (idealLead < 0.f)idealLead += 6.283185307f;

	float cosAngle = clamp(r1v.Dot(r2v) / (r1 * r2), -1.f, 1.f);
	float currentAngle = acosf(cosAngle);
	if (r1v.Cross(r2v) < 0.f) currentAngle = 6.283185307f - currentAngle;

	float diff = fabsf(currentAngle - idealLead);
	if (diff > 3.14159265f) diff = 6.283185307f - diff;

	return diff * (180.f / 3.14159265f);
}

static TransferPlan ScanTOFRange(Vector2 shipPos, Vector2 shipVel, APlanet* target, Vector2 sunPos, Vector2 sunVel, float sunMu,
	float tofMin, float tofMax, int sampleCount)
{
	TransferPlan best;
	best.totalDeltaV = FLT_MAX;

	Vector2 r1 = shipPos - sunPos;

	for (int i = 0; i < sampleCount; ++i)
	{
		float tof = tofMin + (tofMax - tofMin) * i / (float(sampleCount - 1));

		Vector2 r2Abs = target->GetFuturePos(tof);
		Vector2 r2 = r2Abs - sunPos;
		LambertResult lam = SolveLambert(r1, r2, tof, sunMu);
		best.transferAngle = lam.transferAngle;
		if (not lam.valid) continue;

		Vector2 v1Abs = lam.v1 + sunVel;
		Vector2 v2Abs = lam.v2 + sunVel;

		Vector2 targetVelAtArrival = target->GetFutureVelocity(tof);

		float dv1 = (v1Abs - shipVel).Length();
		float dv2 = (targetVelAtArrival - v2Abs).Length();
		float totalDv = dv1 + dv2;

		if (totalDv < best.totalDeltaV)
		{
			best.tof = tof;
			best.v1 = v1Abs;
			best.arrivalPos = r2Abs;
			best.totalDeltaV = totalDv;
			best.valid = true;
		}
	}

	return best;
}

TransferPlan FindBestTransfer(Vector2 shipPos, Vector2 shipVel, APlanet* target, Vector2 sunPos, Vector2 sunVel, float sunMu)
{
	float r1 = (shipPos - sunPos).Length();
	float r2Now = (target->GetCenterPos() - sunPos).Length();
	float hohmannTOF = ComputeHohmannTOF(r1, r2Now, sunMu);

	float tofMin = hohmannTOF * 0.3f;
	float tofMax = hohmannTOF * 2.5f;

	constexpr int coarseSampleCount = 20;
	constexpr int fineSampleCount = 80;
	constexpr float margin = 1.5f;


	TransferPlan coarse = ScanTOFRange(shipPos, shipVel, target, sunPos, sunVel, sunMu, tofMin, tofMax, coarseSampleCount);
	if (not coarse.valid) return coarse;

	float coarseStep = (tofMax - tofMin) / float(coarseSampleCount - 1);


	float fineMin = coarse.tof - coarseStep * margin;
	float fineMax = coarse.tof + coarseStep * margin;

	TransferPlan fine = ScanTOFRange(shipPos, shipVel, target, sunPos, sunVel, sunMu, fineMin, fineMax, fineSampleCount);
	return fine.valid ? fine : coarse;
}


vector<Vector2> PredictTransferPath(Vector2 pos, Vector2 vel, Vector2 sunPos, float sunMu, float tof, int steps)
{
	vector<Vector2> path;
	path.reserve(steps);

	float dt = tof / steps;

	auto ComputeAccel = [&](Vector2 p) -> Vector2
		{
			Vector2 dir = sunPos - p;
			float r2 = dir.LengthSquared();
			float r = sqrtf(r2);

			return dir * (sunMu / (r2 * r));
		};

	for (int i = 0; i < steps; ++i)
	{
		Vector2 k1_x = vel;			// 시작점의 속도
		Vector2 k1_v = ComputeAccel(pos);			// 시작점의 가속도

		// K2
		Vector2 x_k2 = pos + k1_x * (dt* 0.5f);
		Vector2 v_k2 = vel + k1_v * (dt * 0.5f);
		Vector2 k2_x = v_k2;			// K2에서의 속도
		Vector2 k2_v = ComputeAccel(x_k2);			// K2에서의 가속도

		// K3
		Vector2 x_k3 = pos + k2_x * (dt * 0.5f);
		Vector2 v_k3 = vel + k2_v * (dt * 0.5f);
		Vector2 k3_x = v_k3;			// K3에서의 속도
		Vector2 k3_v = ComputeAccel(x_k3);			// K3에서의 가속도

		// K4
		Vector2 x_k4 = pos + k3_x * dt;
		Vector2 v_k4 = vel + k3_v * dt;
		Vector2 k4_x = v_k4;			// K4에서의 속도
		Vector2 k4_v = ComputeAccel(x_k4);			// K4에서의 가속도

		Vector2 newPos = pos + (k1_x + 2.f * k2_x + 2.f * k3_x + k4_x) * (dt / 6.f);
		Vector2 newVel = vel + (k1_v + 2.f * k2_v + 2.f * k3_v + k4_v) * (dt / 6.f);

		pos = newPos;
		vel = newVel;


		path.push_back(pos);
	}

	return path;
}
