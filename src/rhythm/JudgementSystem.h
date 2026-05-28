#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include <span>
#include "FrozenChart.h"

namespace rfs {

	namespace JudgeWindows {
		constexpr int32_t kPerfect = 50; // +-50ms
		constexpr int32_t kGreat = 100;
		constexpr int32_t kGood = 150;
		// greater diff -> miss
	}

	enum class JudgeResult {
		Perfect, Great, Good, Miss
	};

	struct GameplaySnapshot {
		int next_idx = 0;
		std::vector<std::uint8_t> note_resolved; // 0: unresolved, 1: resolved
	};

	struct JudgeCommand {
		int note_index = -1;
		JudgeResult result = JudgeResult::Miss;
		enum class Kind { AutoMiss, TapHit } kind = Kind::AutoMiss;
	};

	class JudgementSystem {
	public:
		static std::vector<JudgeCommand> AdvanceAutoMisses(
			const FrozenChart& chart,
			GameplaySnapshot& snapshot,
			float song_time_ms
		);

		static std::optional<JudgeCommand> JudgeLanePress(
			const FrozenChart& chart,
			GameplaySnapshot& snapshot,
			int lane,
			std::int32_t input_song_time_ms
		);
	};
}