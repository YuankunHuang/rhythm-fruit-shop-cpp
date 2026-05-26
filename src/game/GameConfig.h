#pragma once

#include <algorithm>
#include <cstdint>
#include "../platform/UiFontConfig.h"

using namespace rfs;

namespace GameConfig {
	// Loading
	constexpr float kSpinnerPeriodMs = 800.f;

	// Gameplay
	constexpr uint8_t kLaneCount = 4;
	constexpr float kApproachTimeMs = 1600.f;
	constexpr float kJudgeDisplayMs = 600.f;
	constexpr float kSongEndDelayMs = 2000.f;

	// Layout
	constexpr float kFieldWidthFrac = 0.50f;
	constexpr float kFieldCenterFrac = 0.50f;
	constexpr float kJudgeYFrac = 0.86f;
	constexpr float kSpawnYFrac = 0.08f;
	constexpr float kNoteHFrac = 0.14f;

	// Window/UI
	constexpr float kContentMarginFrac = 0.08f;

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
