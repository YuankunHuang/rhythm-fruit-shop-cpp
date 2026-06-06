#pragma once

#include <cstdint>
#include "JudgeCommand.h"
#include "FrozenChart.h"
#include <span>
#include <vector>

namespace rfs {
	class RuntimeStore final {
	public:
		explicit RuntimeStore(std::size_t note_count);

		void Apply(const JudgeCommand& cmd) noexcept;

		/// <summary>
		/// Called in Update per frame
		/// </summary>
		void AdvancePastMissWindow(
			const FrozenChart& chart,
			std::int32_t judge_time_ms,
			std::int32_t song_offset_ms,
			std::int32_t good_window_ms) noexcept;
		std::size_t NextIdx() const noexcept { return next_idx_; }
		std::span<const std::uint8_t> NoteResolved() const noexcept { return note_resolved_; }

	private:
		std::size_t next_idx_ = 0;
		std::vector<std::uint8_t> note_resolved_;
	};
}