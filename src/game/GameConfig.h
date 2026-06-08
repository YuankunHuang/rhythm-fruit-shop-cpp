#pragma once

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include "../platform/UiFontConfig.h"
#include <string>

using namespace rfs;

namespace GameConfig {

	// Calibration
	static constexpr int32_t kCalibrationSteps[] = {
		-75, -50, -25, 0, 25, 50, 75
	};

	// Loading
	constexpr const char* kFallbackCoverPath = "assets/covers/cover-fallback.png";
	inline std::string ResolveCoverPath(std::string path) {
		if (path.empty()) {
			return kFallbackCoverPath;
		}
		return path;
	}

	constexpr float kSpinnerPeriodMs = 800.f;


	// Char select
	constexpr float kSpeedLevels[] = { 2000.f, 1600.f, 1200.f, 800.f };
	constexpr int kDefaultSpeedIndex = 1; // lv2
	constexpr float kPreviewStartMs = 8000.f;
	constexpr float kPreviewDurationMs = 15'000.f;
	constexpr float kPreviewFadeInDuration = 1000.f;
	constexpr float kPreviewFadeOutDuration = 1000.f;

	// Gameplay
	constexpr uint8_t kLaneCount = 4;
	constexpr float kGameplayLeadInMs = 2500.f;
	constexpr float kPauseResumeCountdownMs = 3000.f;
	constexpr float kJudgeDisplayMs = 500.f;
	constexpr float kSongEndDelayMs = 2000.f;

	// Hit FX (Tap feedback)
	constexpr std::size_t kHitBurstMaxSlots = 16;   // pool capacity; practically unreachable
	constexpr float kHitBurstLifetimeMs = 420.f;    // pool eviction threshold (longest layer + margin)
	constexpr float kLaneFlashMs = 100.f;           // vertical lane flash
	constexpr float kNotePopMs = 220.f;             // note-colored burst quad
	constexpr float kHitCoreMs = 90.f;              // bright core highlight
	constexpr float kHitRingMs = 320.f;             // expanding square ring
	constexpr float kHitSpokeMs = 260.f;            // radial spokes
	constexpr float kHitSweepMs = 280.f;            // judge-line horizontal sweep
	constexpr float kJudgePulseMs = 360.f;          // per-lane judge-line pulse

	// Layout
	constexpr float kFieldWidthFrac = 0.50f;
	constexpr float kFieldCenterFrac = 0.50f;
	constexpr float kJudgeYFrac = 0.95f;
	constexpr float kSpawnYFrac = 0.02f;
	constexpr float kNoteHFrac = 0.14f;

	// Window/UI
	constexpr float kContentMarginFrac = 0.08f;

	// UiLayout conventions:
	// - Positions/sizes: prefer content_*, win_w/win_h fractions, or font_* multiples.
	// - Small constants (<16 ref px): use UiLayout::Px(n), never raw literals.
	// - Do not cap layout with absolute pixel max (e.g. min(w*frac, 480)).

	struct UiLayout {
		float win_w = 0.f;
		float win_h = 0.f;
		float scale = 1.f;
		float font_title = 0.f;
		float font_body = 0.f;
		float font_caption = 0.f;
		float font_hud = 0.f;

		float content_left = 0.f;
		float content_right = 0.f;
		float content_top = 0.f;
		float content_bottom = 0.f;
		float content_center_x = 0.f;
		float content_center_y = 0.f;

		float Px(float ref_px) const { return ref_px * scale; }

		static UiLayout Compute(float win_w, float win_h) {
			UiLayout u{};
			u.win_w = win_w;
			u.win_h = win_h;
			const float sx = win_w / UiFontConfig::kRefWidth;
			const float sy = win_h / UiFontConfig::kRefHeight;
			u.scale = std::min(sx, sy);
			u.font_title = UiFontConfig::kRefHeight * u.scale * UiFontConfig::kTitleFrac;
			u.font_body = UiFontConfig::kRefHeight * u.scale * UiFontConfig::kBodyFrac;
			u.font_caption = UiFontConfig::kRefHeight * u.scale * UiFontConfig::kCaptionFrac;
			u.font_hud = UiFontConfig::kRefHeight * u.scale * UiFontConfig::kHudFrac;

			const float m = kContentMarginFrac;
			u.content_left = win_w * m;
			u.content_right = win_w * (1.f - m);
			u.content_top = win_h * m;
			u.content_bottom = win_h * (1.f - m);
			u.content_center_x = win_w * 0.5f;
			u.content_center_y = win_h * 0.5f;
			return u;
		}
	};
}
