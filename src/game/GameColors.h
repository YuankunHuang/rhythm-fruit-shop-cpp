#pragma once

namespace rfs {
	// All RGBA color constants (0xRRGGBBAA)
	namespace GameColors {
		constexpr uint32_t kLaneBg = 0x22223488;  // subtle dark lane strip
		constexpr uint32_t kLaneLine = 0x44446088;  // lane divider
		constexpr uint32_t kJudgeLine = 0xFFEE66CC;  // gold judge line
		constexpr uint32_t kJudgeGlow = 0xFFEE6622;  // wide glow beneath judge line

		constexpr uint32_t kTextWhite = 0xFFFFFFFF;
		constexpr uint32_t kTextGray = 0xCCCCCCFF;
		constexpr uint32_t kTextError = 0xFF4444FF;
		constexpr uint32_t kTextHint = 0x888888FF;

		constexpr uint32_t kBgClear = 0x1E1E28FF;

		constexpr uint32_t kLoadingMask = 0x1E1E28FF;
		constexpr uint32_t kPanelBg     = 0x2A2A3AAA;
		constexpr uint32_t kOverlayMask = 0x000000BB;

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