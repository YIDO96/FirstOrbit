#include "pch.h"
#include "ASpaceship.h"

#include "Core/InputManager.h"
#include "Core/ResourceManager.h"
#include "GameFramework/Camera.h"
#include "GameFramework/Texture.h"
#include "GameFramework/World.h"
#include "GameFramework/Components/UPhysicsComponent.h"
#include "GameFramework/Components/UAABBColliderComponent.h"
#include "Actor/APlanet.h"

void ASpaceship::Init()
{
	_size = Vector2(12.f, 20.f);
	_type = EActorType::Ship;
	_name = "SpaceShip";

	//UAABBColliderComponent* collider = AddComponent<UAABBColliderComponent>();
	//collider->SetSide(_size);
	//_collider = collider;

	_physicsComp = AddComponent<UPhysicsComponent>();
	SetTexture(RESOURCE.GetTexture(L"Spaceship"));

}
void ASpaceship::Update(float deltaTime)
{
	Super::Update(deltaTime);
	if (_forceHeliocentric)
	{
		_forceHeliocentricTimer -= deltaTime;
		if (_forceHeliocentricTimer <= 0.f)
			_forceHeliocentric = false;
	}


	UpdateTargetPlanet();

	Input(deltaTime);

	// 2) _isThrusting이 true인 프레임에만 deltaTime * _fuelConsumeRate만큼 _fuel을 깎을 것
	if (_isThrusting)
	{
		_fuel -= _fuelConsumeRate * deltaTime;
	}
	// 3) _fuel이 0 밑으로 내려가지 않게 클램프할 것
	_fuel = clamp(_fuel, 0.f, 100.f);


	_trailSampleTimer += deltaTime;
	if (_trailSampleTimer >= 0.05f)
	{
		_trailSampleTimer = 0.f;
		_trail.push_back(GetCenterPos());
		if (_trail.size() > maxTrail)
			_trail.erase(_trail.begin());
	}

	if (abs((-GetCenterPos().y) - _lastSentAltitude) >= 0.05f)
	{
		_lastSentAltitude = (-GetCenterPos().y);

		// 등록된 콜백이 있다면 호출 (이벤트 발송)
		if (_onAltitudeChanged)
		{
			_onAltitudeChanged((-GetCenterPos().y));
		}
	}

	if (abs(_physicsComp->GetVelocity().x - _lastHV) >= 0.05f)
	{
		_lastHV = _physicsComp->GetVelocity().x;

		// 등록된 콜백이 있다면 호출 (이벤트 발송)
		if (_onHVChanged)
		{
			_onHVChanged(_physicsComp->GetVelocity().x);
		}
	}
}
void ASpaceship::Render(HDC hdc)
{
	Super::Render(hdc);

	Camera& cam = _ownerWorld->GetCamera();
	Vector2 screenPos = cam.WorldToScreen(GetCenterPos());
	if (_texture)
	{
		//Vector2 screenPos = cam.WorldToScreen(GetCenterPos());
		Vector2 destSize(cam.WorldToScreenScale(_size.x), cam.WorldToScreenScale(_size.y));
		_texture->RenderRotated(hdc, screenPos, DegreeToRadian(GetDegree()), destSize);

	}

	COLORREF color = RGB(255, 255, 0);
	if (_physicsComp)
	{
		_physicsComp->GetIntegrator() == EIntergrator::ExplicitEuler ? color = RGB(255, 0, 0) :
			_physicsComp->GetIntegrator() == EIntergrator::SemiImplicitEuler? color = RGB(0, 255, 0) :
			_physicsComp->GetIntegrator() == EIntergrator::RK4 ? color = RGB(0, 0, 255) : color = RGB(255, 255, 255);
	}


	for (const Vector2& p : _trail)
	{
		Vector2 s = cam.WorldToScreen(p);
		::SetPixel(hdc, (int)s.x, (int)s.y, color);
	}

}
void ASpaceship::OnGUI()
{
	Super::OnGUI();

	if (_isOpenGUI)
	{
		ImGui::SliderFloat("Thrust", &_thrust, 10.f, 60.f);
		ImGui::SliderFloat("RotSpeed", &_rotSpeed, 30.f, 180.f);

		ImGui::TreePop();
	}

	//const char* name = _name.c_str();
	//
	//if (ImGui::TreeNode(name))
	//{
	//	Super::OnGUI();
	//
	//	ImGui::SliderFloat("Thrust", &_thrust, 10.f, 60.f);
	//	ImGui::SliderFloat("RotSpeed", &_rotSpeed, 30.f, 180.f);
	//
	//
	//	ImGui::TreePop();
	//}
}

void ASpaceship::UpdateTargetPlanet()
{
	if (_forceHeliocentric) return;   // 강제 모드 중엔 SOI 자동 판정 스킵

	APlanet* dominant = nullptr;
	for (int32 i = 0; i < _ownerWorld->GetActorCount(); ++i)
	{
		AActor* actor = _ownerWorld->GetActor(i);
		if (actor->GetType() != EActorType::Planet) continue;

		APlanet* planet = static_cast<APlanet*>(actor);
		if (!dominant || planet->GetMu() > dominant->GetMu())
			dominant = planet;
	}

	// 행성이 없는 월드(LaunchWorld)는 균일중력 그대로 유지
	if (not dominant) return;

	APlanet* current = GetTargetPlanet();

	APlanet* best = nullptr;
	float bestSOI = FLT_MAX;

	for (int32 i = 0; i < _ownerWorld->GetActorCount(); ++i)
	{
		AActor* actor = _ownerWorld->GetActor(i);
		if (actor->GetType() != EActorType::Planet) continue;

		APlanet* planet = static_cast<APlanet*>(actor);
		if (planet == dominant) continue;


		float soi = planet->GetSOIRadius(dominant->GetMu());
		if (soi <= 0.f) continue;

		// 이미 이 행성이 타겟이면 탈줄 기준을 5% 더 넉넉하게 잡아서 경계 진동 방지
		float effectiveSOI = (planet == current) ? soi * 1.05f : soi;

		float distSq = (GetCenterPos() - planet->GetCenterPos()).LengthSquared();
		if (distSq <= effectiveSOI * effectiveSOI and soi < bestSOI)
		{
			best = planet;
			bestSOI = soi;
		}
	}

	SetTargetPlanet(best ? best : dominant); // SOI 밖이면 태양으로 귀속
}

void ASpaceship::Input(float deltaTime)
{
	if (_physicsComp->GetIsPaused()) return;   // 발사 대기 중엔 조작 자체를 막는다

	// Rotate (Right,Left)
	if (_INPUT.GetButtonPressed(KeyType::A) or _INPUT.GetButtonPressed(KeyType::Left)) AddRotation(-_rotSpeed * deltaTime);
	if (_INPUT.GetButtonPressed(KeyType::D) or _INPUT.GetButtonPressed(KeyType::Right)) AddRotation(_rotSpeed * deltaTime);
	

	_isThrusting = (_INPUT.GetButtonPressed(KeyType::W) or _INPUT.GetButtonPressed(KeyType::Up)) and (_fuel > 0.f);
}

void ASpaceship::Reset()
{
	SetRotation(0.f);
	_physicsComp->Reset();
	_isThrusting = false;
	_keyInput = KeyType::L;
	_trail.clear();
	_trailSampleTimer = 0.f;
}

void ASpaceship::SetForceHeliocentric(bool force, float duration)
{
	_forceHeliocentric = force;
	_forceHeliocentricTimer = duration;
}
