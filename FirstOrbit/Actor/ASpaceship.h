#pragma once

#include "GameFramework/AActor.h"
#include "Core/InputManager.h"

class ASpaceship : public AActor
{
	using Super = AActor;
public:
	using FOnAltitudeChanged = function<void(float)>;
	using FOnHVChanged = function<void(float)>;

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;
	virtual void OnGUI() override;
	
	
	void UpdateTargetPlanet();
	void Input(float deltaTime);
	void Reset();
	void SetTargetPlanet(class APlanet* planet) { _targetPlanet = planet; }
	void UpdateAutoPilot(float deltaTime);


	void ReFuelFull() { _fuel = _maxFuel; }
	void IncreaseFuel(float fuel) { _maxFuel += fuel; }

	// 이벤트 구독 함수
	void SetOnAltitudeChanged(FOnAltitudeChanged callback) { _onAltitudeChanged = callback; }
	void SetOnHVChanged(FOnHVChanged callback) { _onHVChanged = callback; }


	void SetAutoPilot(bool b) { _autoPilot = b; }
	bool IsAutoPilot() const { return _autoPilot; }

	void SetForceHeliocentric(bool force, float duration = 0.f);

	class APlanet* GetTargetPlanet() const { return _targetPlanet; }
	float GetThrust() const { return _thrust; }
	bool GetIsThrusting() const { return _isThrusting; }
	KeyType GetKeyInput() const { return _keyInput; }
	float GetFuel() const { return _fuel; }
	float GetMaxFuel() const { return _maxFuel; }
	bool IsForceHeliocentric() const { return _forceHeliocentric; }

private:
	// 임시
	// 로컬 좌표계 기준 도형 (원점 = 기체 중심, -y가 정면)
	Vector2 _localPoints[3] = {
		Vector2( 0.f,  -10.f),   // 코
		Vector2(-6.f,    8.f),   // 왼쪽 꼬리
		Vector2( 6.f,    8.f)    // 오른쪽 꼬리
	};

	FOnAltitudeChanged _onAltitudeChanged = nullptr;
	float _altitude = 0.0f;
	float _lastSentAltitude = -9999.0f;
	FOnHVChanged _onHVChanged = nullptr;
	float _lastHV = -9999.0f;

	KeyType _keyInput = KeyType::L;

	class APlanet* _targetPlanet = nullptr;
	class UPhysicsComponent* _physicsComp = nullptr;
	class UCircleColliderComponent* _circleCollider = nullptr;   // 추가: PickActor가 인식할 수 있도록


	//float _moveSpeed = 100.f;
	float _thrust = 25.f;     // 초당 25픽셀 가속, 가속도 차원 [units/s^2], TWR(Thrust-to-Weight Ratio) 1.75
	float _rotSpeed = 90.f;   // 초당 90도

	bool _isThrusting = false;	// 이번 프레임에 추력을 켰는지 체크


	vector<Vector2> _trail;			// 월드 좌표
	static constexpr int32 maxTrail = 1500;
	float _trailSampleTimer = 0.f;

	// 우주선 연료
	float _fuel = 100.f;
	float _maxFuel = 100.f;
	float _fuelConsumeRate = 1.f;


	bool _forceHeliocentric = false;
    float _forceHeliocentricTimer = 0.f;

	float _autoTargetDegree = 0.f;
	bool _autoPilot = false;

	static constexpr int32 kMinSpeedForTurn = 30;
	static constexpr int32 kInitialKickDegree = 3;
	int32 minAltitudeForTurn = 1000;
};

