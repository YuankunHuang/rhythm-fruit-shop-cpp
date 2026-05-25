#pragma once
#include <cstdint>

namespace rfs {

// All RGBA color constants (0xRRGGBBAA)
namespace GameColors {
    constexpr uint32_t kLaneBg      = 0x22223488;  // subtle dark lane strip
    constexpr uint32_t kLaneLine    = 0x44446088;  // lane divider
    constexpr uint32_t kJudgeLine   = 0xFFEE66CC;  // gold judge line
    constexpr uint32_t kJudgeGlow   = 0xFFEE6622;  // wide glow beneath judge line
    constexpr uint32_t kTextWhite   = 0xFFFFFFFF;
    constexpr uint32_t kTextGray    = 0xCCCCCCFF;
    constexpr uint32_t kTextError   = 0xFF4444FF;
    constexpr uint32_t kTextHint    = 0x888888FF;

    // Per-fruit note colors (index by visual_id % kNoteColorCount)
    constexpr int      kNoteColorCount = 4;
    constexpr uint32_t kNoteColors[kNoteColorCount] = {
        0xFF8833FF,  // orange
        0x33AAFFFF,  // sky-blue
        0xAAFF55FF,  // lime
        0xFF55AAFF,  // pink
    };
}

// Screen-relative layout; recomputed each frame from window dimensions.
// Pure data struct — no dependencies.
struct GameLayout {
    float field_left  = 0.f;
    float field_right = 0.f;
    float judge_y     = 0.f;
    float spawn_y     = 0.f;
    float lane_w      = 0.f;
    float note_h      = 0.f;

    float FieldHeight() const { return judge_y - spawn_y; }
    float LaneX(uint8_t lane) const { return field_left + lane * lane_w; }
    float LaneCenterX(uint8_t lane) const { return LaneX(lane) + lane_w * 0.5f; }

    // lane_count is read from the chart; layout adapts to any lane count.
    static GameLayout Compute(float win_w, float win_h, uint8_t lane_count) {
        if (lane_count < 1) lane_count = 1;
        float fw       = win_w * 0.50f;       // field occupies 50% of screen width
        GameLayout L;
        L.field_left   = (win_w - fw) * 0.5f;
        L.field_right  = L.field_left + fw;
        L.judge_y      = win_h * 0.86f;
        L.spawn_y      = win_h * 0.08f;
        L.lane_w       = fw / static_cast<float>(lane_count);
        L.note_h       = L.lane_w * 0.20f;    // note height proportional to lane width
        return L;
    }
};

} // namespace rfs
