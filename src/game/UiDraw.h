#pragma once
#include "../platform/IRenderer.h"
#include <cstdint>

namespace rfs::UiDraw {
    void CoverFill(IRenderer& r, int handle, float win_w, float win_h, float alpha = 1.f);
    void FullScreenDim(IRenderer& r, float win_w, float win_h, std::uint32_t rgba);
    void RectOutline(IRenderer& r, float x, float y, float w, float h, std::uint32_t color);
    void Underline(IRenderer& r, float cx, float y, float half_w, float thickness, std::uint32_t color);
}