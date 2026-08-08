#include "pch.h"
#include "OrbitalGameMode.h"

#include "GameFramework/Components/UPhysicsComponent.h"

#include "Actor/ASpaceship.h"
#include "Actor/APlanet.h"

void OrbitalGameMode::Update(float deltaTime)
{
	Super::Update(deltaTime);

	if (not _ship) return;
	APlanet* home = _ship->GetTargetPlanet();

	if (not home) return;

	if (_orbitState == EOrbitState::Failed) return;

	// 현재 위치와 속도, 표준 중력변수(mu)
	Vector2 r = _ship->GetCenterPos() - home->GetCenterPos();
	Vector2 v = _ship->GetComponent<UPhysicsComponent>()->GetVelocity();
	float mu = home->GetMu();

	// 비에너지
	_eps = 0.5f * v.LengthSquared() - mu / r.Length();
	// 비각운동량 (h, 단위 질량 당 운동량)
	float h = r.Cross(v);


	_isEscaping = (_eps >= 0.f);
	bool crashedNow = r.Length() < home->GetBodyRadius(); 

	if (not _isEscaping)
	{
		// 장반경 ( 타원 궤도에서 긴 지름의 절반값)
		float a = -mu / (2.f * _eps);
		// 이심률
		_eccentricity = sqrtf(max(0, 1 + (2.f * _eps * h * h) / (mu * mu)));
		// 근지점 & 원지점
		_perigee = a * (1.f - _eccentricity);
		_apogee	 = a * (1.f + _eccentricity);

		_a = a; // ImGui에 표시하기 위해서
	}

	_graceTimer += deltaTime;

	bool doomed = !_isEscaping && (_perigee < home->GetBodyRadius());

	if (_orbitState == EOrbitState::Flying && _graceTimer >= kGracePeriod)   // 아직 결과가 안 정해졌을 때만 판정
	{
		if (crashedNow || _isEscaping)
			_orbitState = EOrbitState::Failed;
		else if (_perigee > (home->GetBodyRadius() + 200.f) && _eccentricity < 0.5f)
			_orbitState = EOrbitState::Cleared;
	}

	_isWarning = doomed;
	 

	if (_orbitState == EOrbitState::Failed)
		_ship->GetComponent<UPhysicsComponent>()->SetPaused(true);

}

void OrbitalGameMode::OnGUI()
{
	Vector2 v = _ship->GetComponent<UPhysicsComponent>()->GetVelocity();

	if (ImGui::TreeNode("GameMode"))
	{
		ImGui::Text("GameMode : OrbitalGameMode");
		Super::OnGUI();

		if (_isWarning)
			ImGui::Text("경고! 경고!");


		const char* stateNames[] = { "Flying", "Cleared", "Failed" };
		ImGui::Text("OrbitState: %s", stateNames[(int32)_orbitState]);

		ImGui::Text("v(속도) : %.2f", v.Length());
		ImGui::Text("eps(비에너지) : %.2f", _eps);
		ImGui::Text("a(장반경) : %.1f", _a);
		ImGui::Text("eccentricity(이심률) : %.1f", _eccentricity);
		ImGui::Text("perigee(근지점) : %.1f", _perigee);
		ImGui::Text("apogee(원지점) : %.1f", _apogee);

		ImGui::TreePop();
	}
}

vector<Vector2> OrbitalGameMode::PredictPath(int steps, float dt) const
{
	vector<Vector2> path;
	if (not _ship) return path;
	APlanet* home = _ship->GetTargetPlanet();
	if (not home) return path;

	OrbitState ghost{ _ship->GetCenterPos(), _ship->GetComponent<UPhysicsComponent>()->GetVelocity() };
	float mu = home->GetMu();
	path.reserve(steps);

	for (int i = 0; i < steps; ++i)
	{
		Vector2 dir = home->GetCenterPos() - ghost.pos;
		float r2 = dir.LengthSquared();
		float r = max(sqrtf(r2), home->GetBodyRadius());

		r2 = r * r;

		Vector2 a = dir * (mu / (r2 * r));

		ghost.vel += a * dt;
		ghost.pos += ghost.vel * dt;

		path.push_back(ghost.pos);
	}


	return path;
}
