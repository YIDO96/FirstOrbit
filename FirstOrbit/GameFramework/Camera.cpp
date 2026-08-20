#include "pch.h"
#include "Camera.h"
#include "Core/GameInstance.h"
#include "Core/InputManager.h"

#include "GameFramework/AActor.h"


void Camera::Init(int width, int height)
{
	_width = width;
	_height = height;
	_half = Vector2(_width / 2.f, _height / 2.f);
	_position = Vector2();
	_isControll = true;
}
void Camera::Update(float deltaTime)
{
	UpdateShake(deltaTime);

	UpdateFollow(deltaTime);

	ImGuiIO& io = ImGui::GetIO();
	if (!_INPUT.IsMouseInsideWindow(GAME.GetGameViewportRect()) or io.WantCaptureMouse)
	{
		_targetZoom = _zoom;
		UpdateViewMatrix();
		return;
	}

	if (_isControll)
	{
		UpdateZoom(deltaTime);
		UpdateDrag();
		
	}
	

	UpdateViewMatrix();
}
void Camera::UpdateViewMatrix()
{
	_view = _view.Translate(_half + _shakeOffset) *
			_view.Scale(_zoom) * 	
			_view.Rotate(_rotation + _manualRotation) *	
			_view.Translate(-_position);

	_invView = _view.Inverse();
}

void Camera::UpdateShake(float deltaTime)
{
	if (_shakeTimer <= 0.f)
	{
		_shakeOffset = Vector2();
		return;
	}

	_shakeTimer -= deltaTime;
	float ratio = max(_shakeTimer / _shakeDuration, 0.f); // 1 -> 0 감쇠

	float angle = (float)(rand() % 360) * (3.141592f / 180.f);
	_shakeOffset = Vector2(cosf(angle), sinf(angle)) * (_shakeMagnitude * ratio);
}

void Camera::AddZoom()
{
	_targetZoom *= (1.f + _wheelStep * _INPUT.GetWheelDelta());

	if (_targetZoom < _minZoom) _targetZoom = _minZoom;
	// 추후 Zoom 상한선도 잡아주기
}
void Camera::UpdateZoom(float deltaTime)
{
	AddZoom();

	// 이미 목표치에 아주 가까워졌다면 더 이상 pow 연산을 하지 않고 고정 (Underflow 방지)
	if (std::abs(_zoom - _targetZoom) < _minZoom)
	{
		_zoom = _targetZoom;
	}
	else
	{
		bool zoomingIn = (_targetZoom > _zoom);

		Vector2 before;
		if (zoomingIn)
		{
			before = WorldToMousePos(_INPUT.GetMousePos());
		}
		
		

		float a = std::exp(-k * deltaTime);
		_zoom *= std::pow(_targetZoom / _zoom, 1.f - a);

		if (zoomingIn)
		{
			UpdateViewMatrix();   // ← _zoom이 바뀌었으니 _view/_invView를 먼저 갱신해야
			// 아래 WorldToMousePos가 "바뀐 줌 기준" 값을 줌

			// 바뀐 줌으로, 같은 화면 좌표가 가리키는 월드 좌표
			Vector2 after = WorldToMousePos(_INPUT.GetMousePos());

			// 대입(=)이 아니라 더하기(+=), 차이만큼만
			_position += (before - after);
		}
	}
}

void Camera::UpdateDrag()
{
	

	if (_INPUT.GetButtonPressed(KeyType::RightMouse))
	{
		_position -= WorldSpaceDelta();
	}
}

void Camera::UpdateFollow(float deltaTime)
{
	if (_followTarget)
	{
		Vector2 targetWorldPos = _followTarget->GetCenterPos();
		Vector2 projected = _isSideView ? Vector2(_galacticZ, targetWorldPos.y) : targetWorldPos;
		_targetPosition = projected + _followOffset;
	}
	else
		_targetPosition = _position;

	float a = std::exp(-_followK * deltaTime);   // 줌이 쓰는 것과 같은 지수감쇠

	_position = Vector2::Lerp(_position, _targetPosition, 1.f - a);   // 위치: 선형
	_rotation = LerpAngle(_rotation, _targetRotation, 1.f - a);       // 각도: 최단경로
	// 줌은 이미 UpdateZoom()에서 로그 보간 중
}

void Camera::AddShake(float duration, float magnitude)
{
	_shakeDuration = duration;
	_shakeTimer = duration;
	_shakeMagnitude = magnitude;
}



void Camera::OnSceneGUI()
{
	if (ImGui::TreeNode("Camera"))
	{
		float vec3Pos[3] = { _position.x, _position.y, 1 };
		if (ImGui::DragFloat3("Position", vec3Pos, 0.1f, -10000.f, 10000.0f, "%.2f"))
		{
			_position.x = vec3Pos[0];
			_position.y = vec3Pos[1];
		}
		ImGui::DragFloat("Zoom", &_zoom, 0.0001f, 0.0001f, 100.0f, "%.4fx");
		ImGui::DragFloat("TargetZoom", &_targetZoom, 0.0001f, 0.0001f, 100.0f, "%.4fx");
		ImGui::Text("Rotation : %.3f rad", _rotation);
		ImGui::DragFloat("TargetRotation", &_targetRotation, 0.1f, -100.f, 100.0f, "%.1f"); // radian값
		ImGui::DragFloat("속도 상수", &k, 0.1f, 1.f, 10.0f, "%.f");
		ImGui::DragFloat("팔로우 상수", &_followK, 0.0001f, 0.0001f, 10.0f, "%.4f");


		//ImGui::Text("Mouse ScreenPos X : %.2f, Y : %.2f", _mousePos.x, _mousePos.y);
		//ImGui::Text("Mouse WorldPos X : %.2f, Y : %.2f", _mouseWorldPos.x, _mouseWorldPos.y);


		ImGui::TreePop();
	}
}



float Camera::WorldToScreenScale(float worldLen) const
{
	return worldLen * _zoom;
}
Vector2 Camera::WorldToScreen(Vector2 worldPos)
{
	return _view.TransformPoint(worldPos);   // 항상 리터럴 변환
}
Vector2 Camera::WorldToScreenAtZ(Vector2 worldPos, float z)
{
	Vector2 projected = _isSideView ? Vector2(z, worldPos.y) : worldPos;
	return _view.TransformPoint(projected);
}
Vector2 Camera::ScreenToWorld(Vector2 screenPos)
{
	return _invView.TransformPoint(screenPos);
}
Vector2 Camera::WorldToMousePos(Vector2 mousePos)
{
	return ScreenToWorld(mousePos / GAME.GetRectRatio());
}
Vector2 Camera::WorldSpaceDelta()
{
	Vector2 delte = _INPUT.GetMouseDelta();
	//float ratio = (float)(GAME.GetGameViewportRect().right - GAME.GetGameViewportRect().left) / GWinSizeX;


	return delte / GAME.GetRectRatio() / _zoom;
}
