#pragma once

#include "GameFramework/AActor.h"

class ASpaceship : public AActor
{
	using Super = AActor;
public:

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;

	void Input(float deltaTime);

	void SetTargetPlanet(class APlanet* planet) { _targetPlanet = planet; }


	APlanet* GetTargetPlanet() const { return _targetPlanet; }
	float GetThrust() const { return _thrust; }
	bool GetIsThrusting() const { return _isThrusting; }

private:
	// 임시
	// 로컬 좌표계 기준 도형 (원점 = 기체 중심, -y가 정면)
	Vector2 _localPoints[3] = {
		Vector2( 0.f,  -10.f),   // 코
		Vector2(-6.f,    8.f),   // 왼쪽 꼬리
		Vector2( 6.f,    8.f)    // 오른쪽 꼬리
	};

	class APlanet* _targetPlanet = nullptr;

	//float _moveSpeed = 100.f;
	float _thrust = 35.f;     // 초당 35픽셀 가속, 가속도 차원 [units/s^2], TWR(Thrust-to-Weight Ratio) 1.75
	float _rotSpeed = 90.f;   // 초당 90도

	bool _isThrusting = false;	// 이번 프레임에 추력을 켰는지 체크
};

