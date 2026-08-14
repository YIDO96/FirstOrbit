#pragma once

struct TransferPlan
{
	float tof = 0.f;
	Vector2 v1;
	Vector2 arrivalPos;
	float totalDeltaV = 0.f;
	bool valid = false;
};

class APlanet;

TransferPlan FindBestTransfer(Vector2 shipPos, Vector2 shipVel, APlanet* target, float sunMu);
vector<Vector2>PredictTransferPath(Vector2 pos, Vector2 vel, Vector2 sunPos, float sunMu, float tof, int steps);