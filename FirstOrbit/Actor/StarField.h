#pragma once

class Camera;

// 월드 좌표에 흩뿌린 별 점들. 카메라를 통과시켜 그리므로 팬·줌·회전이 자연히 따라온다.
class StarField
{
public:
    void Init(int32 count, float range);   // ±range 정사각 영역에 균등 분포
    void Render(HDC hdc, Camera& cam);

private:
    vector<Vector2> _stars;   // 월드 좌표
};
