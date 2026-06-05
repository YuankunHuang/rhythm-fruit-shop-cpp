#pragma once
#include "../platform/IRenderer.h"
#include <cstdint>
#include <string_view>

namespace rfs::UiDraw {
    void CoverFill(IRenderer& r, int handle, float win_w, float win_h, float alpha = 1.f);
    void FullScreenDim(IRenderer& r, float win_w, float win_h, std::uint32_t rgba);
    void RectOutline(IRenderer& r, float x, float y, float w, float h, std::uint32_t color);
    void Underline(IRenderer& r, float cx, float y, float half_w, float thickness, std::uint32_t color);

    // Key-hint: a bracketed key token "[KEY]" followed by an action label, e.g. "[ENTER] Play".
    // The key token uses key_color (gold by convention), the label uses label_color.
    // gap is the horizontal space between the token and the label.

    // Total width of a key-hint, for centering or row layout.
    float MeasureKeyHint(IRenderer& r, TextStyle style,
        std::string_view key, std::string_view label, float gap);

    // Draws the key-hint starting at left_x, vertically centered at y. Returns total width.
    float KeyHint(IRenderer& r, float left_x, float y, TextStyle style,
        std::string_view key, std::string_view label,
        std::uint32_t key_color, std::uint32_t label_color,
        std::uint32_t outline, float gap);
}