#pragma once

namespace rfs {
	// All RGBA color constants (0xRRGGBBAA)
	namespace GameColors {

		inline uint32_t WithAlpha(uint32_t rgba, float a) {
			const auto alpha = static_cast<uint32_t>(std::clamp(a, 0.f, 1.f) * 255.f);
			return (rgba & 0xFFFFFF00u) | alpha;
		}

		inline void TextColorsWithFade(
			uint32_t fill, uint32_t outline,
			float alpha,
			uint32_t& out_fill, uint32_t& out_outline,
			float outline_scale = 0.65f)
		{
			out_fill = WithAlpha(fill, alpha);
			out_outline = WithAlpha(outline, alpha * outline_scale);
		}

		constexpr uint32_t kLaneBg = 0x222234BB;  // subtle dark lane strip
		constexpr uint32_t kLaneLine = 0x44446088;  // lane divider
		constexpr uint32_t kJudgeLine = 0xFFEE66CC;  // gold judge line
		constexpr uint32_t kJudgeGlow = 0xFFEE6622;  // wide glow beneath judge line

		constexpr uint32_t kTextWhite = 0xFFFFFFFF;
		constexpr uint32_t kTextGray = 0xCCCCCCFF;
		constexpr uint32_t kTextError = 0xFF4444FF;
		constexpr uint32_t kTextHint = 0x888888FF;
		constexpr uint32_t kTextGold = 0xFFEE66CC;

		// Gameplay progress
		constexpr uint32_t kProgressTrack = 0xFFFFFF22;
		constexpr uint32_t kProgressFill = 0xFFEE66CC;

		constexpr uint32_t kOutlineBlack = 0x000000FF;

		constexpr uint32_t kBgClear = 0x1E1E28FF;
		constexpr uint32_t kBgBright = 0xfdffed88;

		constexpr uint32_t kLoadingMask = 0x1E1E28FF;
		constexpr uint32_t kPanelBg = 0x2A2A3AAA;
		constexpr uint32_t kOverlayMask = 0x000000BB;

		// ChartSelectScreen selection UI
		constexpr uint32_t kSelectAccent = 0xFFEE66CC; // gold accent (same as kJudgeLine)
		constexpr uint32_t kSelectMuted = 0xAAAAAACC; // unselected list text
		constexpr uint32_t kListScrim = 0x00000088; // left panel dark scrim
		constexpr uint32_t kSpeedPillFill = 0xFFEE6622; // very faint gold fill
		constexpr uint32_t kSpeedPillBorder = 0xFFEE6688; // speed pill border

		constexpr uint32_t kPerfect = 0xFFEE00FF;
		constexpr uint32_t kGreat = 0x88EEFFFF;
		constexpr uint32_t kGood = 0x88FF88FF;
		constexpr uint32_t kMiss = 0xFF4444FF;

		// Per-fruit note colors (index by visual_id % kNoteColorCount)
		constexpr int kNoteColorCount = 4;
		constexpr uint32_t kNoteColors[kNoteColorCount] = {
			0xFF8833FF,  // orange
			0x33AAFFFF,  // sky-blue
			0xAAFF55FF,  // lime
			0xFF55AAFF,  // pink
		};
	}
}