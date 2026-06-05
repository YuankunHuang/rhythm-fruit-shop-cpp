#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include <span>
#include "FrozenChart.h"
#include "JudgeCommandBuffer.h"

namespace rfs {
	namespace JudgeWindows {
		constexpr int32_t kPerfect = 50; // +-50ms
		constexpr int32_t kGreat = 100;
		constexpr int32_t kGood = 150;
		// greater diff -> miss
	}

	struct GameplaySnapshot {
		int next_idx = 0;
		std::vector<std::uint8_t> note_resolved; // 0: unresolved, 1: resolved
	};

	class JudgementSystem {
	public:
		TapCommandBuffer JudgeTaps(
			const FrozenChart& chart,
			std::span<const std::uint8_t> note_resolved,
			int next_idx,
			int lane,
			std::int32_t song_time_ms,
			std::int32_t song_offset_ms = 0) const;

		MissCommandBuffer DetectMisses(
			const FrozenChart& chart,
			std::span<const std::uint8_t> note_resolved,
			int next_idx,
			std::int32_t song_time_ms,
			std::int32_t song_offset_ms = 0) const;
	};
}