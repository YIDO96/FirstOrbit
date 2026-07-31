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


private:
	// 임시
	// 로컬 좌표계 기준 도형 (원점 = 기체 중심, -y가 정면)
	Vector2 _localPoints[3] = {
		Vector2( 0.f,  -10.f),   // 코
		Vector2(-6.f,    8.f),   // 왼쪽 꼬리
		Vector2( 6.f,    8.f)    // 오른쪽 꼬리
	};

	float _moveSpeed = 100.f;
	float _rotSpeed = 90.f;   // 초당 90도

	float _gravity = 9.8f;
};

