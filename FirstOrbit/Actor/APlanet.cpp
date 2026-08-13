#include "pch.h"
#include "APlanet.h"

#include "GameFramework/Camera.h"
#include "GameFramework/World.h"
#include "GameFramework/Texture.h"

#include "GameFramework/Components/UCircleColliderComponent.h"

void APlanet::Init()
{
	_type = EActorType::Planet;
	_circleCollider = AddComponent<UCircleColliderComponent>();
	_collider = _circleCollider;
	
	_texture;
	//collider->Init(this, BALL_SIZE / 2);
	//collider->SetCheckCell(true);
}
void APlanet::Update(float deltaTime)
{
	_meanAnomaly += _orbitSpeed * deltaTime;
	SetCenterPos(ComputePositionAtMeanAnomaly(_meanAnomaly));

	Super::Update(deltaTime);
}
void APlanet::Render(HDC hdc)
{


	Camera& cam = _ownerWorld->GetCamera();
	Vector2 screenPos = cam.WorldToScreen(GetCenterPos());
	float r = cam.WorldToScreenScale(_bodyRadius);

	Super::Render(hdc);

	if (_texture)
	{
		Vector2 destSize(max(r * 2.f, 1.f), max(r * 2.f, 1.f)); // 0px 방지


		Vector2 renderPos = screenPos - destSize / 2.f;
		//_texture->RenderRotated(hdc, screenPos, angle, destSize);
		_texture->Render(hdc, renderPos, Vector2(), Vector2(), destSize);
	}
}
void APlanet::OnGUI()
{
	Super::OnGUI();

	if (_isOpenGUI)
	{
		ImGui::TreePop();
	}
}

void APlanet::Setup(string name, Vector2 orbitCenter, float semiMajorAxis, float orbitSpeed, float mu, float bodyRadius,
	float initialAngle, float eccentricity, float argPeriapsis)
{
	_name = name;
	_orbitCenter = orbitCenter;
	_semiMajorAxis = semiMajorAxis;
	_orbitSpeed = orbitSpeed;
	_mu = mu;
	_bodyRadius = bodyRadius;
	_meanAnomaly = initialAngle;
	_eccentricity = eccentricity;
	_argPeriapsis = argPeriapsis;

	_size = Vector2(_bodyRadius * 2, _bodyRadius * 2);

	SetCenterPos(ComputePositionAtMeanAnomaly(_meanAnomaly));


	_circleCollider->SetRadius(_bodyRadius);
}

Vector2 APlanet::GetVelocity() const
{
	float E = SolveKeplerEquation(_meanAnomaly, _eccentricity);
	float r = _semiMajorAxis * (1.f - _eccentricity * cosf(E));


	float factor = (_orbitSpeed * _semiMajorAxis * _semiMajorAxis) / r;
	float vx_pf = -factor * sinf(E);
	float vy_pf = factor * sqrtf(1.f - _eccentricity * _eccentricity) * cosf(E);

	float c = cosf(_argPeriapsis), s = sinf(_argPeriapsis);

	return Vector2(vx_pf * c - vy_pf * s, vx_pf * s + vy_pf * c);
}

Vector2 APlanet::GetAcceleration() const
{
	Vector2 toCenter = _orbitCenter - GetCenterPos();
	float r2 = toCenter.LengthSquared();
	float r = sqrtf(r2);
	if (r < 0.001f) return Vector2();

	// μ_effective = n²·a³ (역시 μ_sun 없이 n, a로부터 유도) — 진짜 뉴턴 중력이라
	// 기존 원운동 근사(ω²·(center-pos))보다 오히려 더 정확해짐

	float muEff = _orbitSpeed * _orbitSpeed * _semiMajorAxis * _semiMajorAxis * _semiMajorAxis;
	return toCenter * (muEff / (r2 * r));
}

Vector2 APlanet::GetFuturePos(float t) const
{
	return ComputePositionAtMeanAnomaly(_meanAnomaly + _orbitSpeed * t);
}

float APlanet::GetSOIRadius(float parentMu) const
{
	if (parentMu <= 0.f or _mu >= parentMu) return 0.f;

	return _semiMajorAxis * powf(_mu / parentMu, 0.4f);
}

Vector2 APlanet::ComputePositionAtMeanAnomaly(float meanAnomaly) const
{
	float E = SolveKeplerEquation(meanAnomaly, _eccentricity);
	float nu = 2.f * atan2f(sqrtf(1.f + _eccentricity) * sinf(E * 0.5f),
		sqrtf(1.f - _eccentricity) * cosf(E * .5f));
	float r = _semiMajorAxis * (1.f - _eccentricity * cosf(E));

	return _orbitCenter + Vector2(cosf(nu + _argPeriapsis), sinf(nu + _argPeriapsis)) * r;
}
