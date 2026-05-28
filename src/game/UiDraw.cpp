#include "UiDraw.h"
#include <algorithm>

namespace rfs::UiDraw {

    void CoverFill(IRenderer& r, int handle, float win_w, float win_h, float alpha) {
        if (handle < 0) return;
        float tw = win_w, th = win_h;
        r.GetTextureSize(handle, tw, th);
        if (tw <= 0.f || th <= 0.f) return;
        float scale = std::max(win_w / tw, win_h / th);
        float dw = tw * scale, dh = th * scale;
        r.SubmitSprite((win_w - dw) * 0.5f, (win_h - dh) * 0.5f, dw, dh, handle, alpha);
    }

    void FullScreenDim(IRenderer& r, float win_w, float win_h, std::uint32_t rgba) {
        r.SubmitQuad({ 0.f, 0.f, win_w, win_h, rgba });
    }

    void RectOutline(IRenderer& r, float x, float y, float w, float h, std::uint32_t color) {
        r.SubmitLine({ x, y, x + w, y, color });
        r.SubmitLine({ x + w, y, x + w, y + h, color });
        r.SubmitLine({ x + w, y + h, x, y + h, color });
        r.SubmitLine({ x, y + h, x, y, color });
    }

    void Underline(IRenderer& r, float cx, float y, float half_w, float thickness, std::uint32_t color) {
        const float h = std::max(2.f, thickness);
        r.SubmitQuad({ cx - half_w, y, half_w * 2.f, h, color });
    }

}