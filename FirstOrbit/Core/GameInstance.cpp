#include "pch.h"
#include "GameInstance.h"
#include "ImGui/ImGui_ImplGDI.h"

#include "TimeManager.h"
#include "InputManager.h"
#include "Util.h"
#include "ResourceManager.h"
#include "GameFramework/Texture.h"
#include "WorldManager.h"
#include "DataManager.h"
#include "SoundManager.h"
#include "UIManager.h"

#include "Worlds/TitleWorld.h"
#include "Worlds/LaunchWorld.h"
#include "Worlds/MainWorld.h"
#include "Worlds/GameOverWorld.h"
#include "Worlds/EditorWorld.h"


GameInstance::~GameInstance()
{
	// GDI 오브젝트 및 DC 해제
	ReleaseDC(_hwnd, _hdc);
	DeleteDC(_hdcBack);
	DeleteObject(_bmpBack);
	DeleteDC(_hdcGame);
	DeleteObject(_bmpGame);
}

void GameInstance::RecreateBackBuffer(int width, int height)
{
	if (_hdcBack) DeleteDC(_hdcBack);
	if (_bmpBack) DeleteObject(_bmpBack);

	_hdcBack = CreateCompatibleDC(_hdc);

	BITMAPINFO bi = {};
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = -height;   // 음수 = top-down (y가 아래로 증가)
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	_bmpBack = CreateDIBSection(_hdc, &bi, DIB_RGB_COLORS, (void**)&_backPixels, nullptr, 0);

	// 생성된 백버퍼 HDC에 맞는 텍스처를 연결한다.
	HBITMAP prev = (HBITMAP)SelectObject(_hdcBack, _bmpBack);
	DeleteObject(prev);	// 기존에 가지고있던 작은 텍스처는 버린다.
}

void GameInstance::Init(HWND hwnd)
{
	srand((unsigned int)time(0));

	_hwnd = hwnd;

	// 해당 윈도우가 그려지는 메인 HDC 도화지를 얻어오기
	_hdc = GetDC(hwnd);

	// 윈도우 크기를 가져온다. (출력용 도화지와 크기를 맞추기 위해서)
	GetClientRect(hwnd, &_rect);

	// 더블버퍼링 (창 크기와 함께 리사이즈되는 버퍼)
	RecreateBackBuffer(_rect.right, _rect.bottom);

	// 게임 논리 렌더 버퍼: GWinSizeX x GWinSizeY 고정 크기. 창 크기와 무관하다.
	_hdcGame = CreateCompatibleDC(_hdc);
	_bmpGame = CreateCompatibleBitmap(_hdc, GWinSizeX, GWinSizeY);
	SelectObject(_hdcGame, _bmpGame);

	_screenCenter = Vector2(_rect.right / 2.f, _rect.bottom / 2.f);

	// TimeManager
	TimeManager::GetInstance().Init();
	InputManager::GetInstance().Init(hwnd);

	// 리소스 매니저 초기화
	wchar_t buffer[MAX_PATH];
	DWORD length = ::GetCurrentDirectory(MAX_PATH, buffer);
	fs::path currentPath = fs::path(buffer) / L"../Resources/";
	ResourceManager::GetInstance().Init(hwnd, currentPath);

	// SoundManager 초기화 (ResourceManager가 사운드를 로드할 때 디바이스가 이미 있어야 한다)
	SoundManager::GetInstance().Init(hwnd);

	// DataManager 초기화
	DataManager::GetInstance().Init(currentPath);
	DataManager::GetInstance().Load();

	RegisterWorld();

	// 시작World
	WORLD.ChangeWorld("TitleWorld");

	// UIManager는 첫 World가 텍스처를 로드한 "다음"에 초기화해야 관련 텍스처를 찾을 수 있다.
	UIManager::GetInstance().Init();
}

void GameInstance::OnResize(int width, int height)
{
	// Init() 이전(CreateWindowW 도중)에 날아오는 조기 WM_SIZE는 무시한다.
	if (!_hwnd) return;
	if (width <= 0 || height <= 0) return;
	if (width == _rect.right && height == _rect.bottom) return;

	_rect.right = width;
	_rect.bottom = height;

	RecreateBackBuffer(width, height);

	_screenCenter = Vector2(width / 2.f, height / 2.f);

	// 창 크기가 바뀌면 레터박스 뷰포트와 좌표 변환 비율도 즉시 다시 계산한다.
	UpdateGameViewport();
}

void GameInstance::RegisterUIBackgroundRect(int x, int y, int w, int h)
{
	_uiBackgroundRects.push_back({ x, y, x + w, y + h });
}

void GameInstance::SetLaunchHandoff(const LaunchHandoff& handoff)
{
	_launchHandoff = handoff;
	_hasLaunchHandoff = true;
}

bool GameInstance::ConsumeLaunchHandoff(LaunchHandoff& out)
{
	if (!_hasLaunchHandoff) return false;
	out = _launchHandoff;
	_hasLaunchHandoff = false;
	return true;
}

void GameInstance::Update()
{
	// 카메라(마우스 좌표 변환)가 이 값을 쓰므로, 월드 Update보다 먼저 확정해야 한다.
	// Render()에서 계산하면 항상 직전 프레임 값을 쓰게 된다.
	UpdateGameViewport();

	// 각종 업데이트 로직 처리
	TimeManager::GetInstance().Update();
	// 입력 업데이트
	InputManager::GetInstance().Update();

	// F1: 콜라이더 히트박스 시각화 등 디버그 모드 토글
	if (InputManager::GetInstance().GetButtonDown(KeyType::F1))
		_debugMode = !_debugMode;

	// 델타 타임 가져오기
	float deltaTime = TimeManager::GetInstance().GetDT();

	// 현재 월드 업데이트 (액터 업데이트/충돌/GameMode Tick까지 위임)
	WORLD.Update(deltaTime);

	UIManager::GetInstance().Update(deltaTime);

	// 이벤트성 값이기에 안 지울 경우에 한 번 굴렸는데 계속 줌이 들어가게 됨
	//_wheelDelta = 0.f;
	_INPUT.ResetWheelDelta();
}

void GameInstance::Render()
{
	// 현재 월드의 모든 액터를 고정 해상도(GWinSizeX x GWinSizeY) 게임 논리 버퍼에 렌더링한다.
	// 창 크기와 무관하게 항상 같은 좌표계라, World/Collider/Camera 쪽은 전혀 리사이즈를 신경 쓰지 않는다.
	// 우주 배경이라 검정으로 클리어한다 (타일 사이 틈이 생겨도 흰 선으로 드러나지 않는다).
	PatBlt(_hdcGame, 0, 0, GWinSizeX, GWinSizeY, BLACKNESS);
	WORLD.Render(_hdcGame);

	// 인게임 UI(HP 등)는 액터 렌더링 다음, 레터박스 스케일링 전에 같은 버퍼에 이어서 그린다.
	UIManager::GetInstance().Render(_hdcGame);

	// 창 크기에 맞춰 비율을 유지하며 확대/축소(레터박스)해서 백버퍼에 합성한다.
	// 좌상단에 고정하므로, 창이 게임 비율보다 넓거나 높으면 남는 공간이 오른쪽/아래로 생기고
	// 그 자리가 자연스럽게 ImGui 패널 도킹 공간이 된다.
	//float scale = min(_rect.right / (float)GWinSizeX, _rect.bottom / (float)GWinSizeY);
	//int destW = (int)(GWinSizeX * scale);
	//int destH = (int)(GWinSizeY * scale);
	//_gameViewport = { 0, 0, destW, destH };
	//_rectRatio = (float)(_gameViewport.right - _gameViewport.left) / GWinSizeX;;

	int destW = _gameViewport.right - _gameViewport.left;
	int destH = _gameViewport.bottom - _gameViewport.top;

	// 플레이 화면(게임 뷰포트)은 검정으로 남기고, 그 바깥(레터박스 여백)은 흰색으로 채운다.
	PatBlt(_hdcBack, 0, 0, _rect.right, _rect.bottom, WHITENESS);
	if (destW == GWinSizeX && destH == GWinSizeY)
	{
		// 창이 기본 크기일 땐(scale == 1) 그냥 1:1로 복사한다 (StretchBlt보다 훨씬 빠르다).
		::BitBlt(_hdcBack, 0, 0, destW, destH, _hdcGame, 0, 0, SRCCOPY);
	}
	else
	{
		// HALFTONE은 품질은 좋지만 GDI 소프트웨어 래스터라이저에서 프레임당 수십 ms까지 걸릴 만큼
		// 느려서(120fps 목표에 안 맞음), 기본 COLORONCOLOR(최근접 이웃) 모드로 빠르게 확대/축소한다.
		::StretchBlt(_hdcBack, 0, 0, destW, destH, _hdcGame, 0, 0, GWinSizeX, GWinSizeY, SRCCOPY);
	}

	// 이번 프레임에 등록된 ImGui 창 배경들을 ImGui 소프트웨어 래스터라이저 대신 GDI로 빠르게 채운다.
	if (!_uiBackgroundRects.empty())
	{
		static HBRUSH panelBrush = CreateSolidBrush(RGB(15, 15, 15));
		for (const RECT& r : _uiBackgroundRects)
			FillRect(_hdcBack, &r, panelBrush);
		_uiBackgroundRects.clear();
	}

	// 백버퍼 디바이스 컨텍스트(_hdcBack)를 전달하여 ImGui를 메모리 비트맵에 그린다.
	ImDrawData* draw_data = ImGui::GetDrawData();
	if (draw_data)
		ImGui_ImplGDI_RenderDrawData(draw_data, _backPixels, _rect.right, _rect.bottom);

	// 모든 렌더링이 끝난 백버퍼를 화면에 전달
	BitBlt(_hdc, 0, 0, _rect.right, _rect.bottom, _hdcBack, 0, 0, SRCCOPY);
}

void GameInstance::OnGui()
{
	WORLD.OnGUI();
}


void GameInstance::RegisterWorld()
{
	// 씬(World)들을 월드매니저에 등록 (≈ 언리얼의 OpenLevel로 전환 가능한 레벨 목록)
	WORLD.RegisterWorld("TitleWorld", []() {return new TitleWorld(); });
	WORLD.RegisterWorld("LaunchWorld", []() {return new LaunchWorld(); });
	WORLD.RegisterWorld("MainWorld", []() {return new MainWorld(); });
	WORLD.RegisterWorld("GameOverWorld", []() {return new GameOverWorld(); });
	WORLD.RegisterWorld("EditorWorld", []() {return new EditorWorld(); });
}

void GameInstance::UpdateGameViewport()
{
	float scale = min(_rect.right / (float)GWinSizeX, _rect.bottom / (float)GWinSizeY);
	int destW = (int)(GWinSizeX * scale);
	int destH = (int)(GWinSizeY * scale);

	_gameViewport = { 0, 0, destW, destH };
	_rectRatio = (float)(_gameViewport.right - _gameViewport.left) / GWinSizeX;
}
