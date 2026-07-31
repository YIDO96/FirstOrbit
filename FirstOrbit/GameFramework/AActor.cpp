#include "pch.h"
#include "AActor.h"

#include "Components/UActorComponent.h"
#include "Components/UColliderComponent.h"

AActor::~AActor()
{
	for (UActorComponent* c : _components)
		delete c;
	_components.clear();
}


void AActor::SetRotation(float degree)
{
	_degree = degree;

	float radian = DegreeToRadian(degree);

	// 기준(0도) 방향을 매번 다시 회전시켜 계산한다.
	// 기준: 정면 = 화면 위쪽(0,-1), 오른쪽 = (1,0). (y가 아래로 증가하는 좌표계)
	// 이렇게 하면 각도만 단일 소스로 두고 두 방향 벡터가 항상 그로부터 파생된다.
	_forwardDir = Vector2(0, -1).Rotate(radian);
	_rightDir   = Vector2(1,  0).Rotate(radian);
}

void AActor::Update(float deltaTime)
{
	_center = Vector2(_pos.x + _size.x/2, _pos.y +_size.y/2);

	for (UActorComponent* c : _components)
		c->Update(deltaTime);
}

void AActor::UpdateCollider(float deltaTime)
{
	if (_collider)
		_collider->Update();
}

void AActor::Render(HDC hdc)
{
	for (UActorComponent* c : _components)
		c->Render(hdc);
}

void AActor::OnGUI()
{
	for (UActorComponent* c : _components)
		c->OnGUI();
}
