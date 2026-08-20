#include "pch.h"
#include "TitleWorld.h"

#include "Core/InputManager.h"
#include "Core/WorldManager.h"
#include "Core/UIManager.h"
#include "Core/ResourceManager.h"
#include "Core/SoundManager.h"

#include "Core/Sound.h"

#include "Widget/Widget_Title.h"

void TitleWorld::Enter()
{
	Super::Enter();

	LoadTexture();

	_camera.SetIsControll(false);
	_titleWidget = UI.CreateWidget<Widget_Title>();
	_titleWidget->SetOwnerWorld(this);

	_elapsedTime = 0.f;   // BGM과 함께 다시 시작되는 연출이라, 타이틀에 재진입할 때마다 리셋

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
	Super::Render(hdc);

	// BGM의 "삐비비빅 삐 삐비비빅" 구간(대략 4~9초)에 맞춰, 게임 화면 중앙에
	// "First Orbit"을 한 글자씩 타자기처럼 채워나간다. 4초까지는 아무것도 안 그리고,
	// 9초 시점에 전체 문자열이 완성되어 그대로 유지된다.
	constexpr float kTypeStart = 4.f;
	constexpr float kTypeEnd   = 9.f;
	static const wstring kFullText = L"First Orbit";

	if (_elapsedTime < kTypeStart) return;

	float progress = clamp((_elapsedTime - kTypeStart) / (kTypeEnd - kTypeStart), 0.f, 1.f);
	int charCount = (int)std::ceil(progress * (float)kFullText.length());   // ceil이라 4초 시점에 바로 1글자 등장
	charCount = clamp(charCount, 0, (int)kFullText.length());
	if (charCount <= 0) return;

	wstring visible = kFullText.substr(0, charCount);

	// 폰트는 최초 1회만 생성해 계속 재사용한다 (UIButton의 static dim 비트맵과 같은 패턴).
	static HFONT font = ::CreateFontW(56, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		0, 0, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"맑은 고딕");
	HFONT oldFont = (HFONT)::SelectObject(hdc, font);

	::SetBkMode(hdc, TRANSPARENT);
	::SetTextColor(hdc, RGB(255, 255, 255));

	// 전체 문자열 기준으로 시작 x를 고정해서, 글자가 늘어나도 왼쪽이 밀리지 않고
	// 항상 같은 자리에서 오른쪽으로만 채워지게 한다.
	SIZE fullSize;
	::GetTextExtentPoint32W(hdc, kFullText.c_str(), (int)kFullText.length(), &fullSize);
	int startX = (GWinSizeX - fullSize.cx) / 2;
	int y = GWinSizeY / 2 - 100;

	::TextOutW(hdc, startX, y, visible.c_str(), (int)visible.length());

	::SelectObject(hdc, oldFont);
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
