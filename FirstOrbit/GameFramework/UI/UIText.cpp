#include "pch.h"
#include "UIText.h"

void UIText::Init()
{
	
}

void UIText::Update(float deltaTime)
{
}

void UIText::Render(HDC hdc)
{
	if (!_isActive) return;

	Super::Render(hdc);


	::TextOutW(hdc, static_cast<int>(_x), static_cast<int>(_y), _text.c_str(), static_cast<int>(_text.length()));
}

void UIText::OnGUI()
{
	static char textBuffer[120] = "";
	ImGui::Text("Text : ");
	ImGui::SameLine();
	if (ImGui::InputText("##", textBuffer, IM_ARRAYSIZE(textBuffer)))
	{
		_text = CharToWStringStandard(textBuffer);
	}
}
