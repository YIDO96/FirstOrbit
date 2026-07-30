// FirstOrbit.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "pch.h"
#include "framework.h"
#include "FirstOrbit.h"
#include "ImGui/ImGui_ImplGDI.h"
#include "Core/GameInstance.h"

#include "Core/InputManager.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND gHwnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// ImGui Win32 백엔드의 메시지 처리 선언
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParma, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_FIRSTORBIT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // 키보드 내비게이션 활성화
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // 패널을 드래그해서 원하는 위치에 도킹 가능하게

    // 한글이 ??로 깨지지 않도록, 윈도우 기본 한글 폰트(맑은 고딕)로 아틀라스를 굽는다.
    {
        char fontsDir[MAX_PATH];
        ::GetWindowsDirectoryA(fontsDir, MAX_PATH);
        std::string fontPath = std::string(fontsDir) + "\\Fonts\\malgun.ttf";
        io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
    }

    ImGui_ImplWin32_Init(gHwnd);
    ImGui_ImplGDI_Init();


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FIRSTORBIT));

    GameInstance& game = GameInstance::GetInstance();  // 메모리에 할당되면서, 객체 인스턴스
    game.Init(gHwnd);

    MSG msg = {};

    // GDI 소프트웨어 렌더러라 GPU 스왑체인 V-Sync는 없다.
    // 대신 정밀 타이머로 프레임 간격을 120FPS에 맞춰 페이싱한다 (소프트웨어 V-Sync).
    LARGE_INTEGER frequency, now, prev;
    ::QueryPerformanceFrequency(&frequency);
    ::QueryPerformanceCounter(&prev);

    const LONGLONG frameTicks = frequency.QuadPart / 120;   // 120 프레임 목표 간격(틱)

    // Sleep 기본 해상도(~15ms)로는 8.33ms 간격을 못 맞추므로 1ms로 올린다.
    ::timeBeginPeriod(1);

    // 게임은 메세지가 없어도 그림+Update 루틴이 돌아야 한다 (블럭 대기 X).
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            ::QueryPerformanceCounter(&now);
            LONGLONG elapsedTicks = now.QuadPart - prev.QuadPart;

            // 아직 다음 프레임 시각이 안 됐으면 Sleep으로 CPU 양보 (막판 오차는 다음 루프에서).
            if (elapsedTicks < frameTicks)
            {
                double remainMs = (frameTicks - elapsedTicks) * 1000.0 / frequency.QuadPart;
                if (remainMs > 2.0)
                    ::Sleep((DWORD)(remainMs - 1.0));
                continue;
            }

            {
                // 1. ImGui 새 프레임
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                // 창 전체를 덮는 도킹스페이스. 중앙(패스스루)은 투명 통과 → 밑의 게임 렌더가 비쳐 보인다.
                ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

                static bool showDemo = false;
                if (InputManager::GetInstance().GetButtonDown(KeyType::F4))   // F1로 켜고 끄기
                    showDemo = !showDemo;
                if (showDemo)
                    ImGui::ShowDemoWindow(&showDemo);

                static double msUpdate = 0, msGameRender = 0, msViewport = 0;
                double toMs = 1000.0 / frequency.QuadPart;
                LARGE_INTEGER a, b;

                // --- Update 측정 ---
                ::QueryPerformanceCounter(&a);
                game.Update();
                ::QueryPerformanceCounter(&b);
                msUpdate = (b.QuadPart - a.QuadPart) * toMs;

                // --- UI 구성 (지난 프레임 측정값 표시) ---
                ImGui::SetNextWindowPos(ImVec2((float)GWinSizeX, 0.f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2((float)GImGuiPanelWidth, (float)GWinSizeY), ImGuiCond_FirstUseEver);
                ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground);
                {
                    ImVec2 wpos = ImGui::GetWindowPos();
                    ImVec2 wsize = ImGui::GetWindowSize();
                    GameInstance::GetInstance().RegisterUIBackgroundRect((int)wpos.x, (int)wpos.y, (int)wsize.x, (int)wsize.y);
                }
                ImGui::Text("FPS       : %.1f", ImGui::GetIO().Framerate);
                ImGui::Text("Update    : %.2f ms", msUpdate);
                ImGui::Text("GameRender: %.2f ms", msGameRender);
                ImGui::Text("Viewport  : %.2f ms", msViewport);
                ImGui::End();

                game.OnGui();
                ImGui::Render();

                // --- GameRender 측정 ---
                ::QueryPerformanceCounter(&a);
                game.Render();
                ::QueryPerformanceCounter(&b);
                msGameRender = (b.QuadPart - a.QuadPart) * toMs;

                // --- Viewport 측정 ---
                ::QueryPerformanceCounter(&a);
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                ::QueryPerformanceCounter(&b);
                msViewport = (b.QuadPart - a.QuadPart) * toMs;

                // 다음 프레임 기준 시각 갱신 (+= 누적 → Sleep 오차가 안 쌓이고 평균 120FPS 수렴).
                prev.QuadPart += frameTicks;
                if (now.QuadPart - prev.QuadPart > frameTicks * 4)   // 여러 프레임 밀렸으면 따라잡기 폭주 방지
                    prev.QuadPart = now.QuadPart;
            }
        }
    }

    ::timeEndPeriod(1);

    // 프로그램 종료 시 ImGui 해제
    ImGui_ImplGDI_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FIRSTORBIT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_FIRSTORBIT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   // 윈도우 사이즈 결정: 게임 화면(GWinSizeX x GWinSizeY) + 오른쪽 ImGui 고정 패널
   RECT windowRect = { 0, 0, GWinSizeX + GImGuiPanelWidth, GWinSizeY };
   ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, true);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // 생성된 윈도우 핸들을 임시로 저장하자.
   gHwnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // ImGui가 메시지를 먼저 처리하도록 함 (마우스 클릭, 드래그 등)
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message) 
    {
    case WM_MOUSEWHEEL:
        {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            _INPUT.SetWheelDelta(static_cast<float>(zDelta) / static_cast<float>(WHEEL_DELTA));
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
        // 최소화(크기 0)는 무시. GameInstance::Init() 이전에 오는 조기 WM_SIZE는
        // OnResize 내부의 _hwnd 널 체크로 걸러진다.
        if (wParam != SIZE_MINIMIZED)
            GameInstance::GetInstance().OnResize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
