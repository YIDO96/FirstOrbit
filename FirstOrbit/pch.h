#pragma once

#include <iostream>
#include <windows.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <algorithm>
#include <windowsx.h>
#include <queue>
#include <cassert>
#include <numbers>
#include <cmath>


// C++ 20 std::format
#include <format>
#include <filesystem>
namespace fs = std::filesystem;

#include "Core/Util.h"

// transparent Blt
#pragma comment(lib, "msimg32.lib")

// timeBeginPeriod: Sleep 정밀도를 1ms로 올려서 프레임 페이싱 오차를 줄인다.
#pragma comment(lib, "winmm.lib")

// DirectSound (mmioOpen 등 웨이브 파일 파싱도 winmm이 담당한다)
#include <dsound.h>
#pragma comment(lib, "dsound.lib")

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"

// Json
#include <fstream>
#include "Json/nlohmann/json.hpp"
using json = nlohmann::json;




// 정수형을 관리하기 편한 용도로, 별칭을 만들자.
// -> 언리얼이 이런방식이라 미리 익숙해지자.

using int8 = char;		// 1byte 정수형
using int16 = short;	// 2byte 정수형
using int32 = int;		// 4byte 정수형
using int64 = long long;// 8byte 정수형

using uint8 = unsigned char;		// 1byte 정수형
using uint16 = unsigned short;	// 2byte 정수형
using uint32 = unsigned int;		// 4byte 정수형
using uint64 = unsigned long long;// 8byte 정수형



using namespace std;

// Structs.h(Vector2, Matrix3x3 등)가 정규화 없이 vector<...>를 쓰기 때문에,
// using namespace std; 이후에 include해야 한다.
#include "Structs.h"





// 윈도우 사이즈
constexpr int32 GWinSizeX = 600;
constexpr int32 GWinSizeY = 800;

// 한 프레임 델타타임의 상한(초). 창 드래그·중단점·스톨로 dt가 폭증하면
// 액터가 콜라이더를 관통(터널링)하거나 수치적분이 발산하므로 이 값으로 자른다.
// (프레임이 10fps 아래로 떨어지면 게임 시간을 느리게 흐르게 해 안정성을 지킨다)
constexpr float GMaxDeltaTime = 1.0f / 10.0f;

// 게임 화면 오른쪽에 고정으로 붙는 ImGui 패널의 너비
constexpr int32 GImGuiPanelWidth = 300;
