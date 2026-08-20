#pragma once

#include "GameFramework/World.h"

// 타이틀 화면 씬. SpaceBar를 누르면 MainWorld로 전환한다.
class TitleWorld : public World
{
	using Super = World;

public:
	virtual void Enter() override;
	virtual void Update(float deltaTime) override;
	virtual void Render(HDC hdc) override;

protected:
	virtual void OnSceneGUI() override;
	virtual wstring GetResourceSectionName() const override { return L"TitleWorld"; }

private:
	class Widget_Title* _titleWidget = nullptr;

	class Sound* _titleSound = nullptr;

	// 게임 화면(뷰포트) 쪽에 "First Orbit" 타이틀을 타자기처럼 한 글자씩 띄우는 연출.
	// 위젯(UIText 등)은 항상 오른쪽 사이드 패널 버퍼에만 그려지는 구조라, 게임 화면에
	// 뭔가 그리려면 World::Render()를 오버라이드해 GDI로 직접 찍어야 한다.
	float _elapsedTime = 0.f;   // Enter() 이후(= BGM 재생 시작 이후) 경과 시간

	class Texture* _spaceCockpit = nullptr;

	vector<class Texture*> _space;
};
