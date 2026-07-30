#pragma once

#include "UActorComponent.h"

class Texture;

// 정적 스프라이트(회전 포함) 렌더링을 담당하는 컴포넌트.
// Texture는 ResourceManager가 소유(캐시)하는 비소유 포인터라 여기선 빌려서 쓰기만 한다.
class USpriteComponent : public UActorComponent
{
public:
	USpriteComponent(AActor* owner) : UActorComponent(owner) {}

	void SetTexture(Texture* texture) { _texture = texture; }
	Texture* GetTexture() const { return _texture; }

	virtual void Render(HDC hdc) override;

private:
	Texture* _texture = nullptr;
};
