#include "pch.h"
#include "Camera.h"
#include "Core/GameInstance.h"
#include "Core/InputManager.h"
void Camera::Init(int width, int height)
{
	_width = width;
	_height = height;
	_half = Vector2(_width / 2.f, _height / 2.f);
	_position = Vector2();
	
}

void Camera::Update(float deltaTime)
{
	if (!_INPUT.IsMouseInsideWindow(GAME.GetGameViewportRect()))
	{
		UpdateViewMatrix();
		return;
	}

	UpdateZoom(deltaTime);
	UpdateDrag();

	if (_INPUT.GetButtonDown(KeyType::LeftMouse))
	{
		_mousePos = _INPUT.GetMousePos();
	}

	UpdateViewMatrix();
}
void Camera::UpdateViewMatrix()
{
	_view = _view.Translate(_half) *
			_view.Scale(_zoom) * 
			_view.Rotate(_rotation + _manualRotation) *
			_view.Translate(-_position);
	

	_invView = _view.Inverse();
}

void Camera::AddZoom()
{
	_targetZoom *= (1.f + _wheelStep * _INPUT.GetWheelDelta());

	if (_targetZoom < 0.001f) _targetZoom = 0.001f;
}
void Camera::UpdateZoom(float deltaTime)
{
	ImGuiIO& io = ImGui::GetIO();

	if (io.WantCaptureMouse)
	{
		_targetZoom = _zoom;
		return;
	}

	AddZoom();

	// 휠을 사용했다면 (+일때, 줌을 당겼을 때)
	//if (_INPUT.GetWheelDelta() >= 0.0001f)
	//{
	//	_mousePos = WorldToMousePos(_INPUT.GetMousePos());
	//	Vector2 dir = (_mousePos - _position).Normalized();
	//	float len = (_mousePos - _position).Length();
	//	
	//	//_position += dir * len;
	//	_position = Vector2::Lerp(_position, _mousePos, 0.1f);
	//	//_position = Vector2::lerp(_position, targetWorldPos, 0.15f);
	//}

	// 이미 목표치에 아주 가까워졌다면 더 이상 pow 연산을 하지 않고 고정 (Underflow 방지)
	if (std::abs(_zoom - _targetZoom) < 0.00001f)
	{
		_zoom = _targetZoom;
	}
	else
	{
		bool zoomingIn = (_targetZoom > _zoom);

		//float a = std::exp(-k * deltaTime);
		//_zoom *= std::pow(_targetZoom / _zoom, 1.f - a);
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
	

	if (_INPUT.GetButtonPressed(KeyType::LeftMouse))
	{
		_position -= WorldSpaceDelta();
	}
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
		ImGui::DragFloat("Zoom", &_zoom, 0.01f, 0.01f, 100.0f, "%.4fx");
		ImGui::DragFloat("TargetZoom", &_targetZoom, 0.01f, 0.01f, 100.0f, "%.4fx");
		ImGui::DragFloat("Rotation", &_rotation, 0.1f, -100.f, 100.0f, "%.1f"); // radian값
		ImGui::DragFloat("속도상수", &k, 0.1f, 1.f, 10.0f, "%.f");

		ImGui::Text("Mouse ScreenPos X : %.2f, Y : %.2f", _mousePos.x, _mousePos.y);
		ImGui::Text("Mouse WorldPos X : %.2f, Y : %.2f", WorldToMousePos(_mousePos).x, WorldToMousePos(_mousePos).y);


		ImGui::TreePop();
	}
}



float Camera::WorldToScreenScale(float worldLen) const
{
	return worldLen * _zoom;
}
Vector2 Camera::WorldToScreen(Vector2 worldPos)
{
	// 월드 좌표 - 카메라 월드 좌표 + Screen의 사이즈 절반값

	//Vector2 screenPos;
	//Vector2 size = Vector2(_width, _height);
	//
	//
	//screenPos = worldPos - _position + (size / 2);
	//
	//return screenPos;

	return _view.TransformPoint(worldPos);
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
