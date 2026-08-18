#pragma once

class Camera
{

public:
	Camera() = default;
	~Camera() = default;
	void Init(int width, int height);
	void Update(float deltaTime);
	void OnSceneGUI();

	float WorldToScreenScale(float worldLen) const;
	Vector2 WorldToScreen(Vector2 worldPos);  // 월드 좌표를 스크린 좌표로 변환
	Vector2 ScreenToWorld(Vector2 screenPos); // 스크린좌표를 월드 좌표로 변환
	
	Vector2 WorldToMousePos(Vector2 mousePos);
	Vector2 WorldSpaceDelta();

	void AddZoom();
	void UpdateZoom(float deltaTime);
	void UpdateDrag();
	void UpdateFollow(float deltaTime);

	void SetPosition(Vector2 pos) { _position = pos; }
	void SetZoom(float z) { _targetZoom = z; }
	void SetZoomImmediate(float z) { _zoom = _targetZoom = z; }
	void SetRotation(float r) { _rotation = r; }
	void SetIsControll(bool controll) { _isControll = controll; }
	void SetFollowTarget(class AActor* target) { _followTarget = target; }
	void SetTargetPosition(Vector2 pos) { _targetPosition = pos; }
	void SetFollowOffset(Vector2 offset) { _followOffset = offset; }


	Vector2 GetPosition() const { return _position; }
	float GetZoom() const { return _zoom; }
	float GetRotation() const { return _rotation; }
	AActor* GetFollowTarget() const { return _followTarget; }
	const Matrix3x3 GetViewMatrix() const { return _view; }

private:
	void UpdateViewMatrix();




private:
	Vector2 _position;				// 카메라의 World상 중심 좌표
	Vector2 _half;					// 카메라 화면 절반 값
	int _width;						// 화면 너비
	int _height;					// 화면 높이


	float _minZoom = 0.005f;
	float _maxZoom = 10.f;

	float _rotation = 0.f;			// 게임 로직이 전하는 카메라 회전각(라디안)
	float _targetRotation = 0.f;	// _rotation이 서서히 따라갈 목표각 (zoom <-> targetZoom같은구조)
	float _manualRotation = 0.f;	// 디버그용 수동 회전용

	Matrix3x3 _view, _invView;

	Vector2 _mousePos;
	Vector2 _mouseWorldPos;

	class AActor* _followTarget = nullptr;
	Vector2 _targetPosition;
	float _followK = 5.f;
	Vector2 _followOffset = Vector2();

	// Zoom
	float _zoom = 1.f;
	float _targetZoom = 1.f;
	float k = 5.f;					// 지수감쇠에 쓰이는 속도 상수
	float _wheelStep = 0.15f;

	bool _isControll = true;
};
