#pragma once

namespace rfs {
	// Font sizes as a fraction of reference height; scaled uniformly by min(sx, sy).
	namespace UiFontConfig {
		constexpr float kRefWidth = 1280.f;
		constexpr float kRefHeight = 720.f;

		constexpr float kTitleFrac = 0.054f;
		constexpr float kBodyFrac = 0.034f;
		constexpr float kCaptionFrac = 0.026f;
		constexpr float kHudFrac = 0.030f;
		constexpr float kJudgeFrac = 0.038f;
	}
}
