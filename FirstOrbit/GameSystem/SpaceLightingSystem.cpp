#include "pch.h"
#include "SpaceLightingSystem.h"
#include "GameSystem/Glow.h"
#include "GameFramework/Camera.h"

bool SpaceLightingSystem::Initialize(HDC hdc, int width, int height)
{
	_width = width;
	_height = height;

	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	_hDIBDC = CreateCompatibleDC(hdc);
	_hDIBBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&_pPixels, NULL, 0);
	if (not _hDIBBitmap) return false;

	_hOldBitmap = (HBITMAP)SelectObject(_hDIBDC, _hDIBBitmap);
	_pBgPixels = new BYTE[width * height * 4];

	// 텍스처 미리 캐싱 (로딩 시 1회 생성)
	_sunGlow = new Glow();
	_sunGlow->Create(256, RGB(255, 120, 40));

	_engineGlow = new Glow();
	_engineGlow->Create(128, RGB(255, 140, 40));

	_accretionGlow = new Glow();
	//_accretionGlow->Create(256, RGB(255, 120, 30));
	_accretionGlow->Create(256, RGB(255, 255, 255));

	_blackholeGlow = new Glow();
	_blackholeGlow->Create(256, RGB(255, 255, 255));

	_cloudGlow = new Glow();
	_cloudGlow->Create(200, RGB(255, 255, 255));

	return true;
}

void SpaceLightingSystem::Release()
{
	if (_pBgPixels) { delete[] _pBgPixels; _pBgPixels = nullptr; }

	if (_hDIBDC)
	{
		SelectObject(_hDIBDC, _hOldBitmap);
		DeleteObject(_hDIBBitmap);
		DeleteDC(_hDIBDC);
		_hDIBDC = NULL;
	}

	delete _sunGlow;      _sunGlow = nullptr;
	delete _engineGlow;   _engineGlow = nullptr;
	delete _accretionGlow; _accretionGlow = nullptr;
	delete _cloudGlow; _cloudGlow = nullptr;
}

void SpaceLightingSystem::GenerateStarfield(int starCount, float range)
{
	_starsWorld.clear();
	_starsWorld.reserve(starCount);

	for (int i = 0; i < starCount; ++i)
	{
		float x = ((float)rand() / RAND_MAX * 2.f - 1.f) * range;
		float y = ((float)rand() / RAND_MAX * 2.f - 1.f) * range;
		_starsWorld.push_back(Vector2(x, y));
	}
}

void SpaceLightingSystem::BeginRender(Camera& cam)
{
	//// 블랙홀 화면 크기를 여기서 한 번만 계산(클램프 포함)하고, 코어/별 둘 다 이 비율을 같이 쓴다
	//float trueBhScreenRadius = cam.WorldToScreenScale(_blackHoleEventRadius);
	//_bhScreenRadius = clamp(trueBhScreenRadius, 15.f, 80.f);
	//float bhScale = (_blackHoleEventRadius > 0.001f) ? (_bhScreenRadius / _blackHoleEventRadius) : 1.f;
	//Vector2 bhScreenCenter = cam.WorldToScreen(_blackHoleCenter);

	memset(_pBgPixels, 0,  _width * _height * 4);

	// StarField와 동일한 AABB 컷 
	Vector2 c0 = cam.ScreenToWorld(Vector2(0.f, 0.f));
	Vector2 c1 = cam.ScreenToWorld(Vector2((float)_width, 0.f));
	Vector2 c2 = cam.ScreenToWorld(Vector2(0.f, (float)_height));
	Vector2 c3 = cam.ScreenToWorld(Vector2((float)_width, (float)_height));
	float minX = min(min(c0.x, c1.x), min(c2.x, c3.x));
	float maxX = max(max(c0.x, c1.x), max(c2.x, c3.x));
	float minY = min(min(c0.y, c1.y), min(c2.y, c3.y));
	float maxY = max(max(c0.y, c1.y), max(c2.y, c3.y));


	for (const Vector2& star : _starsWorld)
	{
		if (star.x < minX || star.x > maxX || star.y < minY || star.y > maxY)
			continue;

		Vector2 s = cam.WorldToScreen(star);
		int32 x = (int32)s.x;
		int32 y = (int32)s.y;
		if (x < 0 || x >= _width || y < 0 || y >= _height) continue;

		int idx = (y * _width + x) * 4;
		_pBgPixels[idx] = _pBgPixels[idx + 1] = _pBgPixels[idx + 2] = 180;
	}

	//for (const FallingStar& fs : _fallingStars)
	//{
	//	Vector2 pos = _blackHoleCenter + Vector2(cosf(fs.angle), sinf(fs.angle)) * fs.radius;
	//	if (pos.x < minX || pos.x > maxX || pos.y < minY || pos.y > maxY) continue;
	//
	//	Vector2 s = cam.WorldToScreen(pos);
	//	int32 x = (int32)s.x, y = (int32)s.y;
	//	if (x < 0 || x >= _width || y < 0 || y >= _height) continue;
	//
	//	int idx = (y * _width + x) * 4;
	//	_pBgPixels[idx] = _pBgPixels[idx + 1] = _pBgPixels[idx + 2] = 180;
	//}
	// 블랙홀 근처 별들 — 카메라 줌이 아니라 bhScale로 위치 계산 (코어랑 같이 커지고 작아짐)
	//for (const FallingStar& fs : _fallingStars)
	//{
	//	Vector2 offset = Vector2(cosf(fs.angle), sinf(fs.angle)) * fs.radius;
	//	Vector2 s = bhScreenCenter + offset * bhScale;
	//
	//	int32 x = (int32)s.x, y = (int32)s.y;
	//	if (x < 0 || x >= _width || y < 0 || y >= _height) continue;
	//
	//	int idx = (y * _width + x) * 4;
	//	_pBgPixels[idx] = _pBgPixels[idx + 1] = _pBgPixels[idx + 2] = 180;
	//}

	memcpy(_pPixels, _pBgPixels, _width * _height * 4);
}

void SpaceLightingSystem::EndRender(HDC hdcTarget)
{
	BitBlt(hdcTarget, 0, 0, _width, _height, _hDIBDC, 0, 0, SRCCOPY);
}

void SpaceLightingSystem::UpdateStars(float deltaTime)
{
	// 별 전체 (태양 중신으로 아주 느리게 다같이 공전
	constexpr float kMaxRotSpeed = 0.01f;
	constexpr float kOrbitFalloffDist = 50000.f;

	for (Vector2& star : _starsWorld)
	{
		float dist = star.Length();
		float angularSpeed = kMaxRotSpeed * expf(-dist / kOrbitFalloffDist);
		float delta = angularSpeed * deltaTime;

		float c = cosf(delta);
		float s = sinf(delta);
		float x = star.x * c - star.y * s;
		float y = star.x * s + star.y * c;

		star.x = x; star.y = y;
	}


	// 블랙홀 근처 별 (안쪽으로 빨려들며 회전(강착원반처럼), 흡수되면 다시 바깥에서 재생성)
	constexpr float kFallSpeed = 800.f;
	constexpr float kBaseSwirlSpeed = 1.5f;
	for (FallingStar& fs : _fallingStars)
	{
		float swirlSpeed = kBaseSwirlSpeed * (_blackHoleClusterRadius / fs.radius);
		fs.angle += swirlSpeed * deltaTime;
		fs.radius -= kFallSpeed * deltaTime;

		if (fs.radius < _blackHoleEventRadius)
			fs.radius = _blackHoleClusterRadius;
	}
}

void SpaceLightingSystem::RenderSunGlow(const Vector2& pos, float radius, BYTE intensity)
{
	_sunGlow->Render(_hDIBDC, pos, radius, intensity);
}

void SpaceLightingSystem::RenderBlackHoleGlow(const Vector2& pos, float radius, BYTE intensity)
{
	_blackholeGlow->Render(_hDIBDC, pos, radius, intensity);
}

void SpaceLightingSystem::SpawnEngineParticle(const Vector2& pos, const Vector2& dir)
{
	EngineParticle p;
	p.pos = pos;

	// 방향에 퍼짐성 (spread) 부여
	float spread = ((rand() % 100) / 100.f - .5f) * .5f;
	p.velocity = (dir + Vector2(spread, spread)) * (100.f + (rand() % 50));
	p.radius = 10.f + (rand() % 10);
	p.maxLife = .4f + (rand() % 100) / 300.f;
	p.currentLife = p.maxLife;
	p.intensity = 255;

	_particles.push_back(p);
}

void SpaceLightingSystem::UpdateAndRenderParticle(float deltaTime)
{
	for (auto it = _particles.begin(); it != _particles.end();)
	{
		it->currentLife -= deltaTime;
		if (it->currentLife <= 0.0f)
		{
			it = _particles.erase(it);
			continue;
		}
		
		it->pos += it->velocity * deltaTime;


		// 위치 이동 및 크기 확장
		float lifeRatio = it->currentLife / it->maxLife;
		float currentRadius = it->radius * (2.f - lifeRatio);		
		BYTE currentAlpha = (BYTE)(255.f * (lifeRatio * lifeRatio)); // 제곱 감쇠

		_engineGlow->Render(_hDIBDC, it->pos, currentRadius, currentAlpha);

		++it;
	}
}

void SpaceLightingSystem::ApplyPlanetShadow(const Vector2& sunPos, const Vector2& planetPos, float planetRadius)
{
	Vector2 sunToPlanet = planetPos - sunPos;
	float distSunPlanet = sunToPlanet.Length();
	if (distSunPlanet < 0.001f) return;

	Vector2 dir = sunToPlanet / distSunPlanet; // 주축 단위 벡터

	// 그림자가 뻗어나가는 먼 쪽 끝점을 화면 대각선 길이만큼 잡고, 그 주변만 스캔한다
	float maxShadowLen = sqrtf((float)(_width * _width + _height * _height));
	Vector2 farPoint = planetPos + dir * maxShadowLen;

	int minX = (int)max(0.f, min(planetPos.x, farPoint.x) - planetRadius);
	int maxX = (int)min((float)_width, max(planetPos.x, farPoint.x) + planetRadius);
	int minY = (int)max(0.f, min(planetPos.y, farPoint.y) - planetRadius);
	int maxY = (int)min((float)_height, max(planetPos.y, farPoint.y) + planetRadius);

	for (int y = minY; y < maxY; ++y)
	{
		for (int x = minX; x < maxX; ++x)
		{
			Vector2 sunToPixel = Vector2((float)x, (float)y) - sunPos;
			float proj = sunToPixel.Dot(dir);

			//  행성 뒤쪽 영역에 있는지 판병
			if (proj > distSunPlanet)
			{
				// |sunToPixel|^2 = proj^2 + prepDist^2;
				float perpDist = sqrt(max(0.f, sunToPixel.LengthSquared() - proj * proj));

				if (perpDist < planetRadius)
				{
					int idx = (y * _width + x) * 4;
					_pPixels[idx]		= (BYTE)(_pPixels[idx]		* 0.15f);
					_pPixels[idx + 1]	= (BYTE)(_pPixels[idx + 1]	* 0.15f);
					_pPixels[idx + 2]	= (BYTE)(_pPixels[idx + 2]	* 0.15f);
				}
			}
		}
	}
}

void SpaceLightingSystem::ApplyBlackHoleLensing(const Vector2& bhPos, float bhRadius, float distortionStr)
{
	float effectRadius = bhRadius * 3.5f;
	//_debugLastGlowOk = _accretionGlow->Render(_hDIBDC, bhPos, effectRadius * 0.8f, 220);

	

	int minX = (int)max(0.f, bhPos.x - effectRadius);
	int minY = (int)max(0.f, bhPos.y - effectRadius);

	int maxX = (int)min((float)_width, bhPos.x + effectRadius);
	int maxY = (int)min((float)_height, bhPos.y + effectRadius);

	// 1. 배경(별) 왜곡 — 이벤트 호라이즌 바깥 ~ effectRadius
	for (int y = minY; y < maxY; ++y)
	{
		for (int x = minX; x < maxX; ++x)
		{
			Vector2 currentPos((float)x, (float)y);
			Vector2 diff = currentPos - bhPos;
			float dist = diff.Length();

			if (dist < bhRadius || dist >= effectRadius) continue;

			int targetIdx = (y * _width + x) * 4;
			float factor = pow(bhRadius / dist, 1.5f) * distortionStr;

			// effectRadius 근처(바깥쪽 30% 구간)에서 부드럽게 0으로 수렴시켜 경계선을 없앤다
			float edgeFade = 1.f - clamp((dist - effectRadius * 0.7f) / (effectRadius * 0.3f), 0.f, 1.f);
			factor *= edgeFade;

			Vector2 srcPos = currentPos + (diff * (factor / dist));

			int srcX = (int)clamp(srcPos.x, 0.f, (float)_width - 1.f);
			int srcY = (int)clamp(srcPos.y, 0.f, (float)_height - 1.f);
			int srcIdx = (srcY * _width + srcX) * 4;

			_pPixels[targetIdx] = _pBgPixels[srcIdx];
			_pPixels[targetIdx + 1] = _pBgPixels[srcIdx + 1];
			_pPixels[targetIdx + 2] = _pBgPixels[srcIdx + 2];
		}
	}


	//for (int y = minY; y < maxY; ++y)
	//{
	//	for (int x = minX; x < maxX; ++x)
	//	{
	//		Vector2 currentPos((float)x, (float)y);
	//		Vector2 diff = currentPos - bhPos;
	//		float dist = diff.Length();
	//
	//		int targetIdx = (y * _width + x) * 4;
	//
	//		if (dist < bhRadius)
	//		{
	//			_pPixels[targetIdx]		= 0;
	//			_pPixels[targetIdx + 1] = 0;
	//			_pPixels[targetIdx + 2] = 0;
	//		}
	//		else if (dist < effectRadius)
	//		{
	//			float factor = pow(bhRadius / dist, 1.5f) * distortionStr;
	//			Vector2 srcPos = currentPos + (diff * (factor / dist));
	//
	//			int srcX = (int)clamp(srcPos.x, 0.f, (float)_width - 1.f);
	//			int srcY = (int)clamp(srcPos.y, 0.f, (float)_height - 1.f);
	//
	//			int srcIdx = (srcY * _width + srcX) * 4;
	//			_pPixels[targetIdx] = _pBgPixels[srcIdx];
	//			_pPixels[targetIdx + 1] = _pBgPixels[srcIdx + 1];
	//			_pPixels[targetIdx + 2] = _pBgPixels[srcIdx + 2];
	//		}
	//	}
	//}

	// 강착원반(Accretion Disk) 글로우 우선 출력
	_accretionGlow->Render(_hDIBDC, bhPos, effectRadius * 0.8f, 220);

	// 3. 블랙홀 주위를 도는 별들 — 왜곡 루프 뒤에 그려서 안 지워지게 한다
	float bhScale = (_blackHoleEventRadius > 0.001f) ? (bhRadius / _blackHoleEventRadius) : 1.f;
	for (const FallingStar& fs : _fallingStars)
	{
		Vector2 offset = Vector2(cosf(fs.angle), sinf(fs.angle)) * fs.radius;
		Vector2 s = bhPos + offset * bhScale;

		int32 x = (int32)s.x, y = (int32)s.y;
		if (x < 0 || x >= _width || y < 0 || y >= _height) continue;

		int idx = (y * _width + x) * 4;
		_pPixels[idx] = _pPixels[idx + 1] = _pPixels[idx + 2] = 255;
	}


	// 3. 이벤트 호라이즌 — 글로우 위에 검은 코어를 마지막으로 다시 뚫는다
	for (int y = minY; y < maxY; ++y)
	{
		for (int x = minX; x < maxX; ++x)
		{
			if ((Vector2((float)x, (float)y) - bhPos).Length() >= bhRadius) continue;

			int targetIdx = (y * _width + x) * 4;
			_pPixels[targetIdx] = 0;
			_pPixels[targetIdx + 1] = 0;
			_pPixels[targetIdx + 2] = 0;
		}
	}
}

void SpaceLightingSystem::AddStartCluster(Vector2 center, int count, float radius)
{
	_starsWorld.reserve(_starsWorld.size() + count);

	for (int i = 0; i < count; ++i)
	{
		float x = ((float)rand() / RAND_MAX * 2.f - 1.f) * radius;
		float y = ((float)rand() / RAND_MAX * 2.f - 1.f) * radius;

		_starsWorld.push_back(center + Vector2(x, y));
	}
}

void SpaceLightingSystem::AddBlackHoleFallingStars(Vector2 center, int count, float maxRadius, float eventRadius)
{

	_blackHoleCenter = center;
	_blackHoleClusterRadius = maxRadius;
	_blackHoleEventRadius = eventRadius;

	// 일반 별들 (_starsWorld) 중 블랙홀 근처에 있는 애들을 찾아서 옮긴다
	for (auto it = _starsWorld.begin(); it != _starsWorld.end();)
	{
		Vector2 offset = *it - center;
		float dist = offset.Length();

		if (dist < maxRadius)
		{
			FallingStar s;
			s.angle = atan2f(offset.y, offset.x);
			s.radius = max(dist, eventRadius);
			_fallingStars.push_back(s);

			it = _starsWorld.erase(it);
		}
		else
		{
			++it;
		}
	}


	// 2) 목표 개수(count)에서 이미 옮겨온 만큼 뺀 나머지를 새로 채운다
	int remaining = count - (int)_fallingStars.size();
	for (int i = 0; i < remaining; ++i)
	{
		FallingStar s;
		s.angle = ((float)rand() / RAND_MAX) * 6.283185f;
		float t = (float)rand() / RAND_MAX;
		//s.radius = eventRadius + ((float)rand() / RAND_MAX) * (maxRadius - eventRadius);
		s.radius = eventRadius + powf(t, 4.f) * (maxRadius - eventRadius);
		_fallingStars.push_back(s);
	}
}

void SpaceLightingSystem::RenderCloud(const Vector2& pos, float radius, BYTE intensity)
{
	_cloudGlow->Render(_hDIBDC, pos, radius, intensity);
}
