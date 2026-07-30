#pragma once
#include "pch.h"
#include <cstdint>
#include <cmath>

// -----------------------------------------------------------------
// ImGui GDI 소프트웨어 백엔드 (멀티 뷰포트 지원)
//
//  원리:
//   - ImGui의 draw data(삼각형 목록)를 CPU에서 직접 래스터라이즈한다.
//   - 메인 뷰포트: 게임 백버퍼(CreateDIBSection) 픽셀에 직접 블렌딩
//   - 보조 뷰포트(밖으로 드래그된 창): 뷰포트마다 DIB 버퍼를 만들어
//     같은 래스터라이저로 그린 뒤 해당 창 DC에 BitBlt
//
//  멀티 뷰포트는 ImGui "docking 브랜치"가 필요하다.
//  (master 브랜치에서도 이 헤더는 컴파일됨 — 뷰포트 코드만 비활성화)
// -----------------------------------------------------------------

// C++17 inline 변수: 헤더에 둬도 프로그램 전체에 딱 하나만 존재
inline unsigned char* g_ImGuiFontPixels = nullptr;  // RGBA32 폰트 아틀라스 (ImGui 소유)
inline int            g_ImGuiFontW = 0;
inline int            g_ImGuiFontH = 0;

// cross(b-a, p-a) : 부호 있는 넓이의 2배. barycentric 계산용 edge function
inline float ImGdi_Edge(const ImVec2& a, const ImVec2& b, float px, float py)
{
    return (b.x - a.x) * (py - a.y) - (b.y - a.y) * (px - a.x);
}

// 삼각형 하나를 픽셀 버퍼에 래스터라이즈
inline void ImGdi_RasterizeTriangle(
    uint32_t* buf, int bufW, int bufH,
    const ImDrawVert& v0, const ImDrawVert& v1, const ImDrawVert& v2,
    int clipL, int clipT, int clipR, int clipB,
    bool sampleFont)
{
    // 바운딩 박스 (클립 사각형 + 버퍼 크기로 잘라냄)
    int minX = (int)floorf(min(v0.pos.x, min(v1.pos.x, v2.pos.x)));
    int minY = (int)floorf(min(v0.pos.y, min(v1.pos.y, v2.pos.y)));
    int maxX = (int)ceilf(max(v0.pos.x, max(v1.pos.x, v2.pos.x)));
    int maxY = (int)ceilf(max(v0.pos.y, max(v1.pos.y, v2.pos.y)));

    if (clipL > clipR || clipT > clipB) return;   // ← 여기 (클립으로 자르기 직전)


    minX = max(minX, max(clipL, 0));
    minY = max(minY, max(clipT, 0));
    maxX = min(maxX, min(clipR, bufW));
    maxY = min(maxY, min(clipB, bufH));
    if (minX >= maxX || minY >= maxY)
        return;

    // 넓이가 0에 가까운 퇴화 삼각형은 스킵
    float area2 = ImGdi_Edge(v0.pos, v1.pos, v2.pos.x, v2.pos.y);
    if (fabsf(area2) < 1e-6f)
        return;
    float invArea = 1.0f / area2;

    // 정점 컬러 채널 분해 (IM_COL32: R=하위바이트, A=최상위)
    float r0 = (float)((v0.col) & 0xFF), r1 = (float)((v1.col) & 0xFF), r2 = (float)((v2.col) & 0xFF);
    float g0 = (float)((v0.col >> 8) & 0xFF), g1 = (float)((v1.col >> 8) & 0xFF), g2 = (float)((v2.col >> 8) & 0xFF);
    float b0 = (float)((v0.col >> 16) & 0xFF), b1 = (float)((v1.col >> 16) & 0xFF), b2 = (float)((v2.col >> 16) & 0xFF);
    float a0 = (float)((v0.col >> 24) & 0xFF), a1 = (float)((v1.col >> 24) & 0xFF), a2 = (float)((v2.col >> 24) & 0xFF);

    // 삼각형 세 정점 색이 전부 같으면(불투명 UI 사각형, 글자 하나하나가 전부 이 경우다 —
    // 글자는 색은 균일하고 폰트 텍스처의 알파만 픽셀마다 달라짐) 픽셀마다 색을 보간할 필요가 없다.
    bool uniformColor = (v0.col == v1.col) && (v1.col == v2.col);

    // ---- barycentric 가중치(w0, w1)를 픽셀마다 ImGdi_Edge()로 새로 계산하지 않고,
    // x/y로 한 칸씩 움직일 때의 증분값만 누적한다 (고전적인 incremental edge function 래스터라이즈).
    // edge(A,B,px,py)는 px,py에 대한 1차식이라 가능한 최적화이고, invArea도 미리 곱해둬서
    // 픽셀 루프 안에서는 곱셈 없이 덧셈만 하면 된다. Debug(/Od) 빌드에서 특히 효과가 크다
    // (인라인 함수 호출이 실제로 인라이닝되지 않고, 곱셈/뺄셈이 매번 다시 일어나던 부분을 없앤다).
    float dw0dx = -(v2.pos.y - v1.pos.y) * invArea;
    float dw0dy = (v2.pos.x - v1.pos.x) * invArea;
    float dw1dx = -(v0.pos.y - v2.pos.y) * invArea;
    float dw1dy = (v0.pos.x - v2.pos.x) * invArea;

    float rowW0 = ImGdi_Edge(v1.pos, v2.pos, minX + 0.5f, minY + 0.5f) * invArea;
    float rowW1 = ImGdi_Edge(v2.pos, v0.pos, minX + 0.5f, minY + 0.5f) * invArea;

    for (int y = minY; y < maxY; y++)
    {
        uint32_t* row = buf + (size_t)y * bufW;
        float w0 = rowW0;
        float w1 = rowW1;

        for (int x = minX; x < maxX; x++, w0 += dw0dx, w1 += dw1dx)
        {
            float w2 = 1.0f - w0 - w1;
            if (w0 < 0.f || w1 < 0.f || w2 < 0.f)
                continue;   // 삼각형 밖

            // 정점 컬러 보간 (균일 색이면 그냥 그 색을 쓴다)
            float sr, sg, sb, sa;
            if (uniformColor)
            {
                sr = r0; sg = g0; sb = b0; sa = a0 / 255.0f;
            }
            else
            {
                sr = w0 * r0 + w1 * r1 + w2 * r2;
                sg = w0 * g0 + w1 * g1 + w2 * g2;
                sb = w0 * b0 + w1 * b1 + w2 * b2;
                sa = (w0 * a0 + w1 * a1 + w2 * a2) / 255.0f;
            }

            // 폰트 아틀라스 샘플링 (글자든 도형이든 전부 여기를 지나감)
            if (sampleFont && g_ImGuiFontPixels)
            {
                float u = w0 * v0.uv.x + w1 * v1.uv.x + w2 * v2.uv.x;
                float v = w0 * v0.uv.y + w1 * v1.uv.y + w2 * v2.uv.y;

                int tx = (int)(u * g_ImGuiFontW);
                int ty = (int)(v * g_ImGuiFontH);
                tx = max(0, min(tx, g_ImGuiFontW - 1));
                ty = max(0, min(ty, g_ImGuiFontH - 1));

                // 아틀라스는 흰색 RGB + 가변 알파 → 알파만 곱해주면 됨
                unsigned char texA = g_ImGuiFontPixels[((size_t)ty * g_ImGuiFontW + tx) * 4 + 3];
                sa *= texA / 255.0f;
            }

            if (sa <= 0.004f)
                continue;   // 완전 투명이면 스킵

            // 완전 불투명이면 대상 픽셀을 읽어서 블렌딩할 필요 없이 그냥 덮어쓴다.
            // (창 배경/탭바처럼 불투명 단색 사각형이 대부분이라 이 분기가 핫패스)
            if (sa >= 0.996f)
            {
                row[x] = ((uint32_t)sr << 16) | ((uint32_t)sg << 8) | (uint32_t)sb;
                continue;
            }

            // 대상 픽셀(GDI DIB = BGRA 메모리 배치)과 알파블렌딩
            uint32_t dst = row[x];
            float db = (float)(dst & 0xFF);
            float dg = (float)((dst >> 8) & 0xFF);
            float dr = (float)((dst >> 16) & 0xFF);

            uint32_t outR = (uint32_t)(sr * sa + dr * (1.0f - sa));
            uint32_t outG = (uint32_t)(sg * sa + dg * (1.0f - sa));
            uint32_t outB = (uint32_t)(sb * sa + db * (1.0f - sa));

            row[x] = (outR << 16) | (outG << 8) | outB;
        }

        rowW0 += dw0dy;
        rowW1 += dw1dy;
    }
}

// draw data 전체를 픽셀 버퍼에 그린다.
//  backPixels : CreateDIBSection 으로 만든 32bpp top-down 버퍼
inline void ImGui_ImplGDI_RenderDrawData(ImDrawData* draw_data, uint32_t* backPixels, int width, int height)
{
    if (draw_data == nullptr || backPixels == nullptr)
        return;
    if (draw_data->CmdListsCount == 0)
        return;

    // GDI 는 그리기 명령을 배칭하므로, 픽셀을 직접 만지기 전에 반드시 플러시
    ::GdiFlush();

    // 뷰포트 원점 보정: 메인 창은 (0,0), 밖으로 나간 창은 모니터 좌표
    ImVec2 off = draw_data->DisplayPos;

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = draw_data->CmdLists[n];
        const ImDrawVert* vtxBuffer = cmdList->VtxBuffer.Data;
        const ImDrawIdx* idxBuffer = cmdList->IdxBuffer.Data;

        for (int c = 0; c < cmdList->CmdBuffer.Size; c++)
        {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[c];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmdList, pcmd);
                continue;
            }

            // 이 커맨드가 폰트 아틀라스를 쓰는지 (기본 UI는 전부 씀)
            bool sampleFont = (pcmd->GetTexID() == (ImTextureID)1);

            // 커맨드별 클립 사각형
            int clipL = (int)(pcmd->ClipRect.x - off.x);
            int clipT = (int)(pcmd->ClipRect.y - off.y);
            int clipR = (int)(pcmd->ClipRect.z - off.x);
            int clipB = (int)(pcmd->ClipRect.w - off.y);

            const ImDrawVert* vtx = vtxBuffer + pcmd->VtxOffset;

            for (unsigned int i = 0; i < pcmd->ElemCount; i += 3)
            {
                ImDrawVert v0 = vtxBuffer[idxBuffer[pcmd->IdxOffset + i + 0]];
                ImDrawVert v1 = vtxBuffer[idxBuffer[pcmd->IdxOffset + i + 1]];
                ImDrawVert v2 = vtxBuffer[idxBuffer[pcmd->IdxOffset + i + 2]];

                v0.pos.x -= off.x; v0.pos.y -= off.y;
                v1.pos.x -= off.x; v1.pos.y -= off.y;
                v2.pos.x -= off.x; v2.pos.y -= off.y;

                ImGdi_RasterizeTriangle(backPixels, width, height,
                    v0, v1, v2,
                    clipL, clipT, clipR, clipB,
                    sampleFont);
            }
        }
    }
}

// -----------------------------------------------------------------
// 멀티 뷰포트 지원 (docking 브랜치에서만 활성화됨)
//  - 밖으로 드래그된 ImGui 창마다 DIB 버퍼를 만들어 그리고,
//    해당 OS 창의 DC에 BitBlt 한다.
//  - 창 생성/이동/입력은 imgui_impl_win32 가 담당한다.
// -----------------------------------------------------------------
#ifdef IMGUI_HAS_VIEWPORT

struct ImGuiGdiViewportData
{
    HDC       memDC = nullptr;
    HBITMAP   bmp = nullptr;
    HBITMAP   oldBmp = nullptr;
    uint32_t* pixels = nullptr;
    int       w = 0, h = 0;
};

inline void ImGdi_DestroyViewportBuffer(ImGuiGdiViewportData* vd)
{
    if (vd->memDC) { SelectObject(vd->memDC, vd->oldBmp); DeleteDC(vd->memDC); vd->memDC = nullptr; }
    if (vd->bmp) { DeleteObject(vd->bmp); vd->bmp = nullptr; }
    vd->pixels = nullptr;
    vd->w = vd->h = 0;
}

// 뷰포트 크기에 맞는 DIB 버퍼 확보 (크기가 같으면 재사용)
inline void ImGdi_EnsureViewportBuffer(ImGuiGdiViewportData* vd, HWND hwnd, int w, int h)
{
    if (vd->bmp && vd->w == w && vd->h == h)
        return;

    ImGdi_DestroyViewportBuffer(vd);
    if (w <= 0 || h <= 0)
        return;

    HDC wdc = ::GetDC(hwnd);

    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;    // top-down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    vd->bmp = ::CreateDIBSection(wdc, &bi, DIB_RGB_COLORS, (void**)&vd->pixels, nullptr, 0);
    vd->memDC = ::CreateCompatibleDC(wdc);
    vd->oldBmp = (HBITMAP)::SelectObject(vd->memDC, vd->bmp);
    vd->w = w;
    vd->h = h;

    ::ReleaseDC(hwnd, wdc);
}

inline void ImGdi_Renderer_CreateWindow(ImGuiViewport* vp)
{
    vp->RendererUserData = IM_NEW(ImGuiGdiViewportData)();
}

inline void ImGdi_Renderer_DestroyWindow(ImGuiViewport* vp)
{
    if (auto* vd = (ImGuiGdiViewportData*)vp->RendererUserData)
    {
        ImGdi_DestroyViewportBuffer(vd);
        IM_DELETE(vd);
    }
    vp->RendererUserData = nullptr;
}

inline void ImGdi_Renderer_SetWindowSize(ImGuiViewport* vp, ImVec2 size)
{
    // 다음 RenderWindow 에서 EnsureViewportBuffer 가 재생성함
    (void)vp; (void)size;
}

inline void ImGdi_Renderer_RenderWindow(ImGuiViewport* vp, void*)
{
    auto* vd = (ImGuiGdiViewportData*)vp->RendererUserData;
    HWND hwnd = (HWND)vp->PlatformHandleRaw;    // imgui_impl_win32 가 넣어줌
    if (vd == nullptr || hwnd == nullptr)
        return;

    int w = (int)vp->Size.x;
    int h = (int)vp->Size.y;
    ImGdi_EnsureViewportBuffer(vd, hwnd, w, h);
    if (vd->pixels == nullptr)
        return;

    ::GdiFlush();

    // 배경 클리어 (ImGui 기본 테마 톤의 짙은 회색, BGRA)
    if (!(vp->Flags & ImGuiViewportFlags_NoRendererClear))
    {
        uint32_t clear = 0x001E1E1E;
        for (size_t i = 0; i < (size_t)w * h; i++)
            vd->pixels[i] = clear;
    }

    // DisplayPos(=vp->Pos) 보정은 RenderDrawData 내부에서 처리됨
    ImGui_ImplGDI_RenderDrawData(vp->DrawData, vd->pixels, w, h);

    // 완성된 버퍼를 해당 창에 출력
    HDC wdc = ::GetDC(hwnd);
    ::BitBlt(wdc, 0, 0, w, h, vd->memDC, 0, 0, SRCCOPY);
    ::ReleaseDC(hwnd, wdc);
}

inline void ImGdi_Renderer_SwapBuffers(ImGuiViewport*, void*)
{
    // RenderWindow 에서 이미 BitBlt 완료 — 할 일 없음
}

#endif // IMGUI_HAS_VIEWPORT

// -----------------------------------------------------------------
// 초기화 / 종료
// -----------------------------------------------------------------
inline void ImGui_ImplGDI_Init()
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_gdi_software";

    // 폰트 아틀라스를 RGBA32 로 굽고, 포인터만 들고 있는다 (메모리는 ImGui 소유)
    io.Fonts->GetTexDataAsRGBA32(&g_ImGuiFontPixels, &g_ImGuiFontW, &g_ImGuiFontH);
    io.Fonts->SetTexID((ImTextureID)1);

#ifdef IMGUI_HAS_VIEWPORT
    // 멀티 뷰포트 렌더러 콜백 등록
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGuiPlatformIO& pio = ImGui::GetPlatformIO();
        pio.Renderer_CreateWindow = ImGdi_Renderer_CreateWindow;
        pio.Renderer_DestroyWindow = ImGdi_Renderer_DestroyWindow;
        pio.Renderer_SetWindowSize = ImGdi_Renderer_SetWindowSize;
        pio.Renderer_RenderWindow = ImGdi_Renderer_RenderWindow;
        pio.Renderer_SwapBuffers = ImGdi_Renderer_SwapBuffers;
    }
#endif
}

inline void ImGui_ImplGDI_Shutdown()
{
#ifdef IMGUI_HAS_VIEWPORT
    ImGui::DestroyPlatformWindows();    // 뷰포트별 버퍼 정리 (DestroyWindow 콜백 경유)
#endif
    g_ImGuiFontPixels = nullptr;        // 아틀라스 해제는 ImGui::DestroyContext 담당
}