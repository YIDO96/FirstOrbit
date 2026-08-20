#pragma once

#include "GameFramework/UI/Widget.h"

class UIText;
class UIButton;
class UISlider;


class Widget_Launch : public Widget
{
	using Super = Widget;
public:
	virtual void Init() override;
	virtual void Update(float deltaTime) override;

	virtual void Render(HDC hdc) override;

	virtual void OnGUI();

	void SetFuelRatio(float ratio);
	void SetStateText(const std::wstring& text);
	void ReSizeStateText();

	void ResetWidget();

	void BindShip(class ASpaceship* ship);
	
private:
	void OnAltitudeChanged(float newAltitude);
	void OnHVChanged(float newHV);

private:
	UIText* _titleText = nullptr;
	UIText* _launchStateText = nullptr;
	UIText* _altitudeText = nullptr;
	UIText* _horizontalVelocityText = nullptr;
	UIText* _scoreText = nullptr;   // TODO: 점수 시스템 확정되면 실제 값 연결
	UIText* _comboText = nullptr;   // TODO: 콤보 시스템 확정되면 실제 값 연결

	// 키 조작 안내 (정적 텍스트 — 값 갱신 없음)
	UIText* _keyGuideTitle = nullptr;
	UIText* _keyLaunchText = nullptr;
	UIText* _keyResetText = nullptr;

	UISlider* _fuelSlider = nullptr;

	UIButton* _FireButton = nullptr;
	//UIButton* _exitButton = nullptr;

	float _lastHV = -1.f;
	wstring _hvText = L"";

	float _lastAltitude = -1.f;
	wstring _altiText = L"";
};
