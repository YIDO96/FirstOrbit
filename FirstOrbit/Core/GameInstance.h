#pragma once

#include "Singleton.h"

// 앱 생애주기 동안 유지되는 전역 상태를 담당한다 (≈ 언리얼의 UGameInstance).
// 게임 규칙/승패 판정은 GameMode가 담당하고, 여긴 창/백버퍼/서브시스템 초기화와
// 메시지 루프 진입점(Init/Update/Render)만 갖는다.
class GameInstance : public Singleton<GameInstance>
{
	// Singleton 객체를 '친구'로 선언해서 private 접근 가능하게 열어준다.
	friend Singleton<GameInstance>;

public:
	void Init(HWND hwnd);

	void Update();
	void Render();
	void OnGui();

	// 창 크기가 바뀔 때(WM_SIZE) 호출된다. 백버퍼만 새 크기로 재생성하고,
	// 게임 논리 해상도(GWinSizeX x GWinSizeY)는 그대로 유지한다.
	void OnResize(int width, int height);

	HWND GetHwnd() { return _hwnd; }

	// 레터박스된 게임 화면이 실제로 그려지는 창 좌표 (마우스 좌표를 게임 좌표로 변환할 때 사용)
	RECT GetGameViewportRect() const { return _gameViewport; }
	float GetRectRatio() const { return _rectRatio; }

	// 자유롭게 움직이는(도킹 가능한) ImGui 창이 자기 배경을 NoBackground로 끄는 대신,
	// Begin() 직후 자신의 현재 위치/크기를 등록해두면 GDI FillRect로 훨씬 빠르게 채워준다.
	// (ImGui의 소프트웨어 삼각형 래스터라이저로 큰 배경을 매 프레임 채우면 비용이 크다)
	void RegisterUIBackgroundRect(int x, int y, int w, int h);

	// F1로 토글되는 디버그 모드 (콜라이더 히트박스 시각화 등)
	bool IsDebugMode() const { return _debugMode; }
	void SetDebugMode(bool b) { _debugMode = b; }
private:
	// 아무나 생성못하게 생성자/소멸자를 숨기자
	GameInstance() = default;
	~GameInstance();

	// _hdcBack/_bmpBack/_backPixels를 지정한 크기로 (재)생성한다.
	void RecreateBackBuffer(int width, int height);

	void RegisterWorld();

	void UpdateGameViewport();

private:
	HWND _hwnd;	// 윈도우 핸들
	RECT _rect;		// 윈도우 크기

	HDC _hdc;		// 메인 도화지 (출력용)
	HDC _hdcBack;	// 실시간으로 그려지는 버퍼 (창 크기와 함께 리사이즈됨)
	HBITMAP _bmpBack;	// back hdc가 사용하는 텍스처

	uint32_t* _backPixels = nullptr;  // DIB 픽셀 직접 접근용 (소유권은 _bmpBack)

	// 게임 논리 렌더 버퍼: GWinSizeX x GWinSizeY 고정 크기. 창 크기와 무관하게 절대 리사이즈되지 않는다.
	// World는 항상 여기 그리고, Render()에서 창 크기에 맞춰 비율 유지 스케일(레터박스)로 합성한다.
	HDC _hdcGame = nullptr;
	HBITMAP _bmpGame = nullptr;

	// 레터박스된 게임 화면이 그려지는 창 좌표 (좌상단 고정, 비율 유지 확대/축소)
	RECT _gameViewport = {};
	float _rectRatio = 1.f;

	// 이번 프레임에 등록된 UI 배경 사각형들 (Render()에서 GDI로 채운 뒤 비운다)
	vector<RECT> _uiBackgroundRects;

	Vector2 _screenCenter;

	bool _debugMode = false;
};



#define GAME GameInstance::GetInstance()
