#include "pch.h"
#include "TitleWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/UIManager.h"
#include "Core/ResourceManager.h"
#include "Core/SoundManager.h"

#include "Core/Sound.h"

#include "GameFramework/Texture.h"

#include "Widget/Widget_Title.h"

void TitleWorld::Enter()
{
	Super::Enter();

	LoadTexture();

	_camera.SetIsControll(false);
	_titleWidget = UI.CreateWidget<Widget_Title>();
	_titleWidget->SetOwnerWorld(this);

	_elapsedTime = 0.f;   // BGM과 함께 다시 시작되는 연출이라, 타이틀에 재진입할 때마다 리셋

	_spaceCockpit = RESOURCE.GetTexture(L"SpaceCockpit");

	_space.clear(); // Enter() 재진입을 고려해 기존 요소 초기화
	_space.reserve(9);
	_space.push_back(RESOURCE.GetTexture(L"Sun"));
	_space.push_back(RESOURCE.GetTexture(L"Mercury"));
	_space.push_back(RESOURCE.GetTexture(L"Venus"));
	_space.push_back(RESOURCE.GetTexture(L"Earth"));
	_space.push_back(RESOURCE.GetTexture(L"Mars"));
	_space.push_back(RESOURCE.GetTexture(L"Jupiter"));
	_space.push_back(RESOURCE.GetTexture(L"Uranus"));
	_space.push_back(RESOURCE.GetTexture(L"Neptune"));
	_space.push_back(RESOURCE.GetTexture(L"Galaxy"));

	//RESOURCE.LoadSound(L"S_Main", L"Sound_Main.wav");
	SOUND.Play(L"S_Main", true);
}

void TitleWorld::Update(float deltaTime)
{
	Super::Update(deltaTime);

	_elapsedTime += deltaTime;

	if (_INPUT.GetButtonDown(KeyType::P))
	{
		WORLD.ChangeWorld("LaunchWorld");
		SOUND.Stop(L"S_Main");
	}

	//if (_INPUT.GetButtonDown(KeyType::F1))
	//	WORLD.ChangeWorld("EditorWorld");

	
}

void TitleWorld::Render(HDC hdc)
{
	//Super::Render(hdc);
	//
	//// BGM의 "삐비비빅 삐 삐비비빅" 구간(대략 4~9초)에 맞춰, 게임 화면 중앙에
	//// "First Orbit"을 한 글자씩 타자기처럼 채워나간다. 4초까지는 아무것도 안 그리고,
	//// 9초 시점에 전체 문자열이 완성되어 그대로 유지된다.
	//constexpr float kTypeStart = 3.5f;
	//constexpr float kTypeEnd   = 7.5f;
	//static const wstring kFullText = L"First Orbit";
	//
	//if (_elapsedTime < kTypeStart) return;
	//
	//float progress = clamp((_elapsedTime - kTypeStart) / (kTypeEnd - kTypeStart), 0.f, 1.f);
	//int charCount = (int)std::ceil(progress * (float)kFullText.length());   // ceil이라 4초 시점에 바로 1글자 등장
	//charCount = clamp(charCount, 0, (int)kFullText.length());
	//if (charCount <= 0) return;
	//
	//wstring visible = kFullText.substr(0, charCount);
	//
	//// 폰트는 최초 1회만 생성해 계속 재사용한다 (UIButton의 static dim 비트맵과 같은 패턴).
	//static HFONT font = ::CreateFontW(56, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
	//	0, 0, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");
	//HFONT oldFont = (HFONT)::SelectObject(hdc, font);
	//
	//::SetBkMode(hdc, TRANSPARENT);
	//::SetTextColor(hdc, RGB(255, 255, 255));
	//
	//// 전체 문자열 기준으로 시작 x를 고정해서, 글자가 늘어나도 왼쪽이 밀리지 않고
	//// 항상 같은 자리에서 오른쪽으로만 채워지게 한다.
	//SIZE fullSize;
	//::GetTextExtentPoint32W(hdc, kFullText.c_str(), (int)kFullText.length(), &fullSize);
	//int startX = (GWinSizeX - fullSize.cx) / 2;
	//int y = GWinSizeY / 2 - 100;
	//
	//::TextOutW(hdc, startX, y, visible.c_str(), (int)visible.length());
	//
	//::SelectObject(hdc, oldFont);



	Super::Render(hdc);

	static const wstring kFullText = L"First Orbit";

	// 1. 박자에 맞춘 글자 수별 등장 시간(초) 정의
	// "Fir(3) s (5) tO(7) r (9) bit(11)" 리듬에 맞춘 타임스탬프
	static const float kTimeStamps[] = {
		3.45f, // 1: 'F'  
		3.60f, // 2: 'i'   |- 빠빠빠 (Fir)
		3.75f, // 3: 'r'  /
		4.50f, // 4: 's'  -- 빠     (s)
		5.10f, // 5: 't'  \  빠빠   (tO)
		5.20f, // 6: ' '  /  (공백 포함)
		5.35f, // 7: 'O' 
		6.15f, // 8: 'r'  -- 빠     (r)
		6.70f, // 9: 'b'  
		6.85f, // 10:'i'   |- 빠빠빠 (bit)
		7.10f  // 11:'t'  /  <-- 요 11번째 't'가 들어가는 타임스탬프가 없어서 안 나왔던 것입니다!
	};

	constexpr float kFadeOutStart = 7.5f; // 페이드 아웃 시작 시간
	constexpr float kFadeOutEnd = 8.5f; // 완전히 사라지는 시간

	if (_elapsedTime >= kFadeOutEnd)
	{
		float spaceTime = _elapsedTime - kFadeOutEnd;

		constexpr float kPlanetDuration = 7.0f;

		// 태양 등장 시각 2.0초로 수정 및 이후 타임라인 1초씩 전체 연장
		static const float kStartTimes[] = {
			2.0f,   // 태양 (Cockpit 등장 2초 후)
			6.0f,   // 수성
			9.8f,   // 금성
			13.8f,  // 지구
			17.8f,  // 화성
			22.5f,  // 목성
			27.2f,  // 토성
			32.5f,  // 천왕성
			37.8f   // 해왕성
		};

		constexpr float kAllPlanetsEndTime = 37.8f + kPlanetDuration; // 약 42.3초

		// ---------------------------------------------------------------------
		// 1. 우리은하 (Galaxy) 배경
		// ---------------------------------------------------------------------
		Texture* galaxy = RESOURCE.GetTexture(L"Galaxy");
		if (galaxy)
		{
			Vector2 galOrigSize = galaxy->GetTextureSize();

			float srcX = 0.0f;
			float srcY = 0.0f;
			float srcW = galOrigSize.x;
			float srcH = galOrigSize.y;

			// 🎯 블랙홀 위치 추가 상향 조정 (Y축 22% -> 18% 지점으로 더 올려서 배치)
			Vector2 galaxyCoreCenter = Vector2((float)GWinSizeX * 0.50f, (float)GWinSizeY * 0.18f);

			float galaxyScale = 1.0f;
			Vector2 destSize = Vector2((float)GWinSizeX, (float)GWinSizeY);
			Vector2 destPos = Vector2(0.f, (float)GWinSizeY * 0.25f - (destSize.y * 0.5f));

			// [상태 A] 행성들이 지나가는 동안
			if (spaceTime < kAllPlanetsEndTime)
			{
				float bgMoveProgress = 0.0f;
				if (spaceTime >= 2.0f)
				{
					bgMoveProgress = (spaceTime - 2.0f) / (kAllPlanetsEndTime - 2.0f);
				}

				float shiftX = bgMoveProgress * (galOrigSize.x * 0.03f);
				float shiftY = bgMoveProgress * (galOrigSize.y * 0.02f);

				srcX = shiftX;
				srcY = (galOrigSize.y * 0.65f) - shiftY;
				srcW = galOrigSize.x * 0.35f;
				srcH = galOrigSize.y * 0.35f;
			}
			// [상태 B] 모든 행성이 지난 후 (은하 및 중심점 확대)
			else
			{
				float postTime = spaceTime - kAllPlanetsEndTime;
				constexpr float kTransitionDuration = 4.0f;

				float progress = clamp(postTime / kTransitionDuration, 0.0f, 1.0f);
				progress = progress * progress * (3.0f - 2.0f * progress);

				float lastShiftX = galOrigSize.x * 0.03f;
				float lastShiftY = galOrigSize.y * 0.02f;

				srcX = (1.0f - progress) * lastShiftX;
				srcY = (1.0f - progress) * (galOrigSize.y * 0.65f - lastShiftY);
				srcW = galOrigSize.x * (0.35f + progress * 0.65f);
				srcH = galOrigSize.y * (0.35f + progress * 0.65f);

				// 은하 확대 스케일 연산
				constexpr float kScaleSpeed = 0.015f;
				galaxyScale = 1.0f + (postTime * kScaleSpeed);

				destSize = Vector2((float)GWinSizeX, (float)GWinSizeY) * galaxyScale;
				destPos = galaxyCoreCenter - (destSize * 0.5f);
			}

			// 은하 배경 렌더링
			galaxy->Render(hdc, destPos, Vector2(srcX, srcY), Vector2(srcW, srcH), destSize);

			// -----------------------------------------------------------------
			// 🕳️ 블랙홀 렌더링 (행성 이탈 완료 후 20초 뒤 등장)
			// -----------------------------------------------------------------
			constexpr float kBlackHoleStartDelay = 20.0f; // 💡 20초 추가 지연 설정
			if (spaceTime >= (kAllPlanetsEndTime + kBlackHoleStartDelay))
			{
				float blackHoleTime = spaceTime - (kAllPlanetsEndTime + kBlackHoleStartDelay);

				// 초기 아주 작은 점(1.0px)에서 시작하여 은하 스케일 증가에 따라 천천히 확대
				constexpr float kBaseDotRadius = 1.0f;
				constexpr float kGrowthFactor = 15.0f;

				float blackHoleRadius = kBaseDotRadius + (blackHoleTime * kGrowthFactor * 0.015f);

				HBRUSH blackBrush = ::CreateSolidBrush(RGB(0, 0, 0));
				HPEN blackPen = ::CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

				HBRUSH oldBrush = (HBRUSH)::SelectObject(hdc, blackBrush);
				HPEN oldPen = (HPEN)::SelectObject(hdc, blackPen);

				// 조정된 Y축 18% 높이(galaxyCoreCenter)에 블랙홀 렌더링
				int left = static_cast<int>(galaxyCoreCenter.x - blackHoleRadius);
				int top = static_cast<int>(galaxyCoreCenter.y - blackHoleRadius);
				int right = static_cast<int>(galaxyCoreCenter.x + blackHoleRadius);
				int bottom = static_cast<int>(galaxyCoreCenter.y + blackHoleRadius);

				::Ellipse(hdc, left, top, right, bottom);

				::SelectObject(hdc, oldBrush);
				::SelectObject(hdc, oldPen);
				::DeleteObject(blackBrush);
				::DeleteObject(blackPen);
			}
		}

		// ---------------------------------------------------------------------
		// 2. 행성 이동 연출 (좌/우 구역 분리 + 높이 축소 + 가속도/오버랩)
		// ---------------------------------------------------------------------
		// [조절] 행성별 생성 위치 (X: 좌/우 분리, Y: 높이 범위 축소 0.22~0.30)
		static const Vector2 kSpawnOffsets[] = {
			Vector2(0.42f, 0.25f), // 0: 태양
			Vector2(0.58f, 0.24f), // 1: 수성
			Vector2(0.35f, 0.28f), // 2: 금성
			Vector2(0.65f, 0.27f), // 3: 지구
			Vector2(0.38f, 0.23f), // 4: 화성
			Vector2(0.62f, 0.29f), // 5: 목성
			Vector2(0.32f, 0.26f), // 6: 토성
			Vector2(0.68f, 0.22f), // 7: 천왕성
			Vector2(0.40f, 0.30f)  // 8: 해왕성
		};

		// [조절] 행성 이동 속도 늦춤 (수명 4.0초 -> 5.5초, 간격 2.6초 -> 3.5초)
		//constexpr float kPlanetDuration = 5.5f;
		constexpr float kSpawnInterval = 3.5f;

		for (size_t i = 0; i < _space.size(); ++i)
		{
			Texture* planet = _space[i];
			if (!planet || i >= sizeof(kSpawnOffsets) / sizeof(kSpawnOffsets[0])) continue;
			if (planet == galaxy) continue;

			float startTime = 2.0f + (i * kSpawnInterval);
			if (spaceTime < startTime) continue;

			float localTime = spaceTime - startTime;
			if (localTime > kPlanetDuration) continue;

			float linearProgress = localTime / kPlanetDuration;

			// 가속 곡선 (3승)
			float accelProgress = linearProgress * linearProgress * linearProgress;

			float scale = 0.01f + (accelProgress * 1.49f);
			Vector2 origSize = planet->GetTextureSize();
			Vector2 renderSize = origSize * scale;

			Vector2 startPos = Vector2((float)GWinSizeX * kSpawnOffsets[i].x, (float)GWinSizeY * kSpawnOffsets[i].y);

			float dirX = (i % 2 == 0) ? -1.0f : 1.0f;
			float offsetX = dirX * (accelProgress * 650.f);
			float offsetY = accelProgress * 350.f;

			Vector2 currentPos = startPos + Vector2(offsetX, offsetY);
			Vector2 drawPos = currentPos - (renderSize * 0.5f);

			planet->Render(hdc, drawPos, Vector2(0.f, 0.f), origSize, renderSize);
		}

		// ---------------------------------------------------------------------
		// 3. 조종석 (_spaceCockpit)
		// ---------------------------------------------------------------------
		if (_spaceCockpit != nullptr)
		{
			_spaceCockpit->Render(
				hdc,
				Vector2(0.f, 0.f),
				Vector2(0.f, 0.f),
				_spaceCockpit->GetTextureSize(),
				Vector2((float)GWinSizeX, (float)GWinSizeY)
			);
		}
	}

	// 3.5초 이전이거나 8.5초 이후면 아무것도 그리지 않음
	if (_elapsedTime < kTimeStamps[0] || _elapsedTime >= kFadeOutEnd) return;

	// 1. 현재 보여줄 글자 수 계산
	int charCount = 0;
	int totalSteps = sizeof(kTimeStamps) / sizeof(kTimeStamps[0]);

	for (int i = 0; i < totalSteps; ++i)
	{
		if (_elapsedTime >= kTimeStamps[i]) charCount = i + 1;
		else break;
	}

	charCount = clamp(charCount, 0, (int)kFullText.length());
	if (charCount <= 0) return;

	wstring visible = kFullText.substr(0, charCount);

	// 2. 투명도(Alpha) 계산 (255: 불투명 -> 0: 완전 투명)
	BYTE alpha = 255;
	if (_elapsedTime >= kFadeOutStart)
	{
		float fadeProgress = (_elapsedTime - kFadeOutStart) / (kFadeOutEnd - kFadeOutStart);
		fadeProgress = clamp(fadeProgress, 0.0f, 1.0f);
		alpha = static_cast<BYTE>((1.0f - fadeProgress) * 255.0f);
	}

	// 3. 폰트 및 위치 설정 (최초 1회만 계산)
	static HFONT font = ::CreateFontW(56, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		0, 0, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");
	static int startX = 0;
	static int startY = 0;
	static SIZE fullSize = { 0, 0 };
	static bool isInitialized = false;

	if (!isInitialized)
	{
		HFONT tempFont = (HFONT)::SelectObject(hdc, font);
		::GetTextExtentPoint32W(hdc, kFullText.c_str(), (int)kFullText.length(), &fullSize);
		startX = (GWinSizeX - fullSize.cx) / 2;
		startY = GWinSizeY / 2 - 100;
		::SelectObject(hdc, tempFont);
		isInitialized = true;
	}

	// 4. AlphaBlend를 위한 메모리 DC 및 비트맵 생성 후 텍스트 그리기
	HDC memDC = ::CreateCompatibleDC(hdc);
	HBITMAP memBitmap = ::CreateCompatibleBitmap(hdc, fullSize.cx, fullSize.cy);
	HBITMAP oldBitmap = (HBITMAP)::SelectObject(memDC, memBitmap);

	// 배경을 검은색(RGB 0,0,0)으로 비우기
	HBRUSH blackBrush = (HBRUSH)::GetStockObject(BLACK_BRUSH);
	RECT textRect = { 0, 0, fullSize.cx, fullSize.cy };
	::FillRect(memDC, &textRect, blackBrush);

	// 메모리 DC에 텍스트 그리기
	HFONT oldFont = (HFONT)::SelectObject(memDC, font);
	::SetBkMode(memDC, TRANSPARENT);
	::SetTextColor(memDC, RGB(255, 255, 255)); // 흰색 글씨
	::TextOutW(memDC, 0, 0, visible.c_str(), (int)visible.length());

	// 5. 알파 블렌딩 적용하여 화면(hdc)에 복사
	BLENDFUNCTION blend = {};
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = alpha; // 계산된 투명도 적용
	blend.AlphaFormat = 0;

	::AlphaBlend(hdc, startX, startY, fullSize.cx, fullSize.cy,
		memDC, 0, 0, fullSize.cx, fullSize.cy, blend);

	// 6. 자원 해제
	::SelectObject(memDC, oldFont);
	::SelectObject(memDC, oldBitmap);
	::DeleteObject(memBitmap);
	::DeleteDC(memDC);
}

void TitleWorld::OnSceneGUI()
{
	//ImGui::Text("F1 : EditorWorld로 진입");
	ImGui::Text("P : 게임 시작");

	if (ImGui::Button("Widget Editor"))
	{
		_titleWidget->SetActiveEidtMode(_isWidgetEditToggle);

		_isWidgetEditToggle = !_isWidgetEditToggle;
	}

}
