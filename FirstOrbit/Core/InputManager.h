#pragma once

#include "Singleton.h"

// 키보드&마우스의 입력 처리
enum class KeyType
{
	LeftMouse = VK_LBUTTON,
	RightMouse = VK_RBUTTON,

	Up = VK_UP,
	Down = VK_DOWN,
	Left = VK_LEFT,
	Right = VK_RIGHT,
	SpaceBar = VK_SPACE,

	KEY_1 = '1',
	KEY_2 = '2',

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',
	L = 'L',
	Q = 'Q',
	E = 'E',
	G = 'G',
	R = 'R',

	F1 = VK_F1,
	F2 = VK_F2,
	F3 = VK_F3,
	F4 = VK_F4,
	F5 = VK_F5,

	Tab = VK_TAB,
	
};

enum class KeyState
{
	None,
	Press,
	Down,
	Up,

	End
};

constexpr int32 KEY_TYPE_COUNT = static_cast<int32>(UINT8_MAX) + 1;

class InputManager : public Singleton<InputManager>
{
	// Singleton 객체를 '친구'로 선언해서 private 접근 가능하게 열어준다.
	friend Singleton<InputManager>;

public:
	void Init(HWND hwnd);
	void Update();

	void SetWheelDelta(float wheelDelta) { _wheelDelta += wheelDelta; }
	void ResetWheelDelta() { _wheelDelta = 0.f; }

	// 누르고 있을 때
	bool GetButtonPressed(KeyType key) { return GetState(key) == KeyState::Press; }

	// 맨 처음 눌렀을 때
	bool GetButtonDown(KeyType key) { return GetState(key) == KeyState::Down; }

	// 맨 처음 눌렀다가 땔 때
	bool GetButtonUp(KeyType key) { return GetState(key) == KeyState::Up; }

	bool IsMouseInsideWindow(RECT viewport);

	// 현재 마우스 위치
	POINT GetMousePos() { return _mousePos; }
	POINT GetPrevMousePos() { return _prevMousePos; }
	POINT GetMouseDelta() { return POINT(_mousePos.x - _prevMousePos.x, _mousePos.y - _prevMousePos.y); }
	
	float GetWheelDelta() { return _wheelDelta; }


private:
	KeyState GetState(KeyType key) { return _states[static_cast<uint8>(key)]; }

private:
	// 아무나 생성못하게 생성자/소멸자를 숨기자
	InputManager() = default;
	~InputManager() = default;


	HWND _hwnd = 0;
	vector<KeyState> _states;
	POINT _mousePos = {};
	POINT _prevMousePos = {};
	float _wheelDelta = 0.f;
};

#define _INPUT InputManager::GetInstance()
