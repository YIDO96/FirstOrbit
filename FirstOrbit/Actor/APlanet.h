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

	void Setup(string name, Vector2 orbitCenter, float orbitRadius, float orbitSpeed, float mu, float bodyRadius, float initialAngle = 0.0f);
	void SetTexture(class Texture* texture) { _texture = texture; }

	
	float GetBodyRadius() const { return _bodyRadius; }
	float GetMu() const { return _mu; }
private:
	Vector2 _orbitCenter;		// 공전 중심점의 월드좌표
	float _orbitRadius = 0;		// 공전 중심에서 행성까지의 거리
	float _orbitSpeed = 0;		// 공전 각속도
	float _angle = 0;			// 공전 각도(라디안)
	float _mu = 0;			// 행성 질량
	float _bodyRadius = 0;		// 행성 자체의 반지름

	class UCircleColliderComponent* _circleCollider = nullptr;
	class Texture* _texture = nullptr;
};