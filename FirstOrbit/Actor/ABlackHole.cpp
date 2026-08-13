#include "pch.h"
#include "ABlackHole.h"

void ABlackHole::Init()
{
	_type = EActorType::BlackHole;
	_name = "BlackHole";
}

void ABlackHole::Setup(Vector2 pos, float mu, float eventHorizonRadius)
{
	SetCenterPos(pos);
	_mu = mu;
	_eventHorizonRadius = eventHorizonRadius;
}

void ABlackHole::Render(HDC hdc)
{

}

void ABlackHole::OnGUI()
{
	Super::OnGUI();

	if (_isOpenGUI)
	{

		ImGui::TreePop();
	}
}


