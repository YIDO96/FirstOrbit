#pragma once

#include "GameFramework/AActor.h"

class APlanet : public AActor
{
	using Super = AActor;

public:

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;

	void Setup(string name, Vector2 orbitCenter, float semiMajorAxis, float orbitSpeed, float mu, float bodyRadius, 
				float initialAngle = 0.0f, float eccentricity = 0.f, float argPeriapsis = 0.f);
	//void SetTexture(class Texture* texture) { _texture = texture; }

	
	float GetBodyRadius() const { return _bodyRadius; }
	float GetOrbitSpeed() const { return _orbitSpeed; }
	float GetMu() const { return _mu; }
	Vector2 GetVelocity() const;
	Vector2 GetAcceleration() const;
	Vector2 GetFuturePos(float t) const;   // t초 뒤 위치를 즉시 계산 (닫힌 형태, 오차 없음)
	Vector2 GetFutureVelocity(float t) const;
	float GetSOIRadius(float parentMu) const;
	vector<Vector2> GetOrbitShape(int steps) const;


private:
	Vector2 ComputePositionAtMeanAnomaly(float meanAnomaly) const;
	Vector2 ComputePositionAtEccentricAnomaly(float E) const;
	Vector2 ComputeVelocityAtEccentricAnomaly(float E) const;


private:
	Vector2 _orbitCenter;		// 공전 중심점의 월드좌표
	float _semiMajorAxis = 0;	// a (기존의 _orbitRadius)
	//float _orbitRadius = 0;	// 공전 중심에서 행성까지의 거리
	float _orbitSpeed = 0;		// 공전 각속도
	float _eccentricity = 0;	// e (0이면 원)
	float _argPeriapsis = 0;	// w, 근일점 방향
	float _meanAnomaly = 0;		// M, 기존 _angle
	//float _angle = 0;			// 공전 각도(라디안)
	float _mu = 0;			// 행성 질량
	float _bodyRadius = 0;		// 행성 자체의 반지름

	class UCircleColliderComponent* _circleCollider = nullptr;
	//class Texture* _texture = nullptr;

	
};