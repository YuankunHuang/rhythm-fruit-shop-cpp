#include "UiDraw.h"
#include <algorithm>
#include <string>

namespace rfs::UiDraw {

    namespace {
        std::string BracketKey(std::string_view key) {
            std::string token;
            token.reserve(key.size() + 2);
            token += '[';
            token.append(key);
            token += ']';
            return token;
        }
    }

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

    float MeasureKeyHint(IRenderer& r, TextStyle style,
        std::string_view key, std::string_view label, float gap)
    {
        const std::string token = BracketKey(key);
        const float key_w = r.MeasureTextWidth(token, style);
        const float label_w = label.empty() ? 0.f : r.MeasureTextWidth(label, style);
        const float mid_gap = label.empty() ? 0.f : gap;
        return key_w + mid_gap + label_w;
    }

    float KeyHint(IRenderer& r, float left_x, float y, TextStyle style,
        std::string_view key, std::string_view label,
        std::uint32_t key_color, std::uint32_t label_color,
        std::uint32_t outline, float gap)
    {
        const std::string token = BracketKey(key);
        const float key_w = r.MeasureTextWidth(token, style);

        r.SubmitText({ left_x, y, Anchor::CenterLeft, style, token, key_color, outline });

        if (label.empty()) {
            return key_w;
        }

        const float label_x = left_x + key_w + gap;
        r.SubmitText({ label_x, y, Anchor::CenterLeft, style, std::string(label), label_color, outline });

        const float label_w = r.MeasureTextWidth(label, style);
        return key_w + gap + label_w;
    }

}