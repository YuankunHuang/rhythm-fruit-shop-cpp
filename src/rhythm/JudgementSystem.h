#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include <span>
#include "FrozenChart.h"
#include "JudgeCommandBuffer.h"

namespace rfs {

	struct JudgementConfig {
		std::int32_t perfect_window_ms = 50;
		std::int32_t great_window_ms = 100;
		std::int32_t good_window_ms = 150;
	};

	class JudgementSystem {
	public:
		explicit JudgementSystem(JudgementConfig config = {}) noexcept : config_(config) {}

		const JudgementConfig& Config() const noexcept { return config_; }

		TapCommandBuffer JudgeTaps(
			const FrozenChart& chart,
			std::span<const std::uint8_t> note_resolved,
			std::size_t next_idx,
			int lane,
			std::int32_t song_time_ms,
			std::int32_t song_offset_ms = 0) const;

		MissCommandBuffer DetectMisses(
			const FrozenChart& chart,
			std::span<const std::uint8_t> note_resolved,
			std::size_t next_idx,
			std::int32_t song_time_ms,
			std::int32_t song_offset_ms = 0) const;

	private:
		JudgementConfig config_;
	};
}