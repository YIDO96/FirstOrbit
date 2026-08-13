#include "pch.h"
#include "StarField.h"
#include "GameFramework/Camera.h"

void StarField::Init(int32 count, float range)
{
    _stars.clear();
    _stars.reserve(count);

    for (int32 i = 0; i < count; ++i)
    {
        float x = ((float)rand() / RAND_MAX * 2.f - 1.f) * range;
        float y = ((float)rand() / RAND_MAX * 2.f - 1.f) * range;
        _stars.push_back(Vector2(x, y));
    }
}

void StarField::Render(HDC hdc, Camera& cam, float brightness)
{
    if (brightness <= 0.01f) return;

    // 화면 네 모서리를 월드로 역변환 → 보일 수 있는 월드 영역(AABB)을 한 번만 구한다
    Vector2 c0 = cam.ScreenToWorld(Vector2(0.f, 0.f));
    Vector2 c1 = cam.ScreenToWorld(Vector2((float)GWinSizeX, 0.f));
    Vector2 c2 = cam.ScreenToWorld(Vector2(0.f, (float)GWinSizeY));
    Vector2 c3 = cam.ScreenToWorld(Vector2((float)GWinSizeX, (float)GWinSizeY));

    float minX = min(min(c0.x, c1.x), min(c2.x, c3.x));
    float maxX = max(max(c0.x, c1.x), max(c2.x, c3.x));
    float minY = min(min(c0.y, c1.y), min(c2.y, c3.y));
    float maxY = max(max(c0.y, c1.y), max(c2.y, c3.y));


    for (const Vector2& star : _stars)
    {

        // 행렬 변환 전에 비교 4번으로 먼저 컷
        if (star.x < minX || star.x > maxX || star.y < minY || star.y > maxY)
            continue;

        Vector2 s = cam.WorldToScreen(star);

        ::SetPixel(hdc, (int)s.x, (int)s.y, RGB(255, 255, 255));
    }
}