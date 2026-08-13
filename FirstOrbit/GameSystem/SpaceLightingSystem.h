#pragma once

struct EngineParticle
{
	Vector2		pos;
	Vector2		velocity;
	float		radius;
	float		maxLife;
	float		currentLife;
	BYTE		intensity;
};

struct FallingStar
{
	float angle;
	float radius;
};

class Glow;
class Camera;

class SpaceLightingSystem
{
public:
	SpaceLightingSystem() = default;
	~SpaceLightingSystem() { Release(); }

	bool Initialize(HDC hdc, int width, int height);
	void Release();

	void GenerateStarfield(int starCount, float range);
	void BeginRender(Camera& cam);
	void EndRender(HDC hdcTarget);

	void UpdateStars(float deltaTime);

	// Glow 및 파티클 연출
	void RenderSunGlow(const Vector2& pos, float radius, BYTE intensity = 255);
	void RenderBlackHoleGlow(const Vector2& pos, float radius, BYTE intensity = 255);
	void SpawnEngineParticle(const Vector2& pos, const Vector2& dir);
	void UpdateAndRenderParticle(float deltaTime);


	// 행성 그림자
	void ApplyPlanetShadow(const Vector2& sunPos, const Vector2& planetPos, float planetRadius);

	// 블랙혹 중력 렌즈 효과
	void ApplyBlackHoleLensing(const Vector2& bhPos, float bhRadius, float distortionStr = 15.f);
	void AddStartCluster(Vector2 center, int count, float radius);
	void AddBlackHoleFallingStars(Vector2 center, int count, float maxRadius, float eventRadius);


	HDC GetBufferDC() const { return _hDIBDC; }
	bool GetDebugLastGlowOk() const { return _debugLastGlowOk; }   // TEMP DEBUG
	float GetBlackHoleScreenRadius() const { return _bhScreenRadius; }

private:
	int _width = 0;
	int _height = 0;

	HDC _hDIBDC = NULL;
	HBITMAP _hDIBBitmap = NULL;
	HBITMAP _hOldBitmap = NULL;

	BYTE* _pPixels = nullptr;		// 메인 렌더 버퍼
	BYTE* _pBgPixels = nullptr;		// 배경 별빛 백업 버퍼

	Vector2 _blackHoleCenter;
	float _blackHoleClusterRadius = 0.f;
	float _blackHoleEventRadius = 0.f;
	vector<FallingStar> _fallingStars;


	// 캐싱된 발광 텍스처 객체들
	Glow* _sunGlow = nullptr;
	Glow* _engineGlow = nullptr;
	Glow* _accretionGlow = nullptr; // 블랙홀 강착원반용
	Glow* _blackholeGlow = nullptr;

	// 엔진 파티클 컨테이너
	vector<EngineParticle> _particles;
	vector<Vector2> _starsWorld;


	bool _debugLastGlowOk = true;   // TEMP DEBUG
	float _bhScreenRadius = 0.f;
};

