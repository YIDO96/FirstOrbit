#pragma once

#include "GameFramework/AActor.h"

struct TrailPoint
{
	Vector2 worldPos;
	float z;
};

struct OrbitShapePoint
{
	Vector2 pos;
	float timeOffset; // 이 위상에 도달하는 시간차(초). 음수=과거, 양수=미래
};

class APlanet : public AActor
{
	using Super = AActor;

public:

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;

	void RecordTrailPoint(float z);
	const deque<TrailPoint>& GetTrail() const { return _trail; }


	void Setup(string name, Vector2 orbitCenter, float semiMajorAxis, float orbitSpeed, float mu, float bodyRadius, 
				float initialAngle = 0.0f, float eccentricity = 0.f, float argPeriapsis = 0.f);
	//void SetTexture(class Texture* texture) { _texture = texture; }
	void SetOrbitCenter(Vector2 center, class APlanet* centerBody = nullptr);
	void RefreshOrbitCenter() { if (_orbitCenterBody) _orbitCenter = _orbitCenterBody->GetCenterPos(); }

	void SetIsMoon(bool isMoon) { _isMoon = isMoon; }
	bool IsMoon() const { return _isMoon; }
	class APlanet* GetOrbitCenterBody() const { return _orbitCenterBody; }

	float GetBodyRadius() const { return _bodyRadius; }
	float GetOrbitSpeed() const { return _orbitSpeed; }
	float GetMu() const { return _mu; }
	Vector2 GetVelocity() const;
	Vector2 GetAcceleration() const;
	Vector2 GetFuturePos(float t) const;   // t초 뒤 위치를 즉시 계산 (닫힌 형태, 오차 없음)
	Vector2 GetFutureVelocity(float t) const;
	float GetSOIRadius(float parentMu) const;
	vector<Vector2> GetOrbitShape(int steps) const;
	vector<OrbitShapePoint> GetOrbitShapeTimed(int steps) const; // 신규 (사이드뷰용)

private:
	Vector2 ComputePositionAtMeanAnomaly(float meanAnomaly) const;
	Vector2 ComputePositionAtEccentricAnomaly(float E) const;
	Vector2 ComputeVelocityAtEccentricAnomaly(float E) const;


private:
	class APlanet* _orbitCenterBody = nullptr;
	bool _isMoon = false;		// true면 SOI 자동타겟팅/호만전이 스캔에서 제외 (아직 계층 구조 미지원)

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

	deque<TrailPoint> _trail;
	static constexpr size_t kMaxTrailPoints = 6000;	// 0.1초 간격으로 기록 (10분치)
};