#include "RuntimeStore.h"

namespace rfs {
	RuntimeStore::RuntimeStore(std::size_t note_count)
		: note_resolved_(note_count, 0)
	{
	}

	void RuntimeStore::Apply(const JudgeCommand& cmd) noexcept {
		if (cmd.note_index >= note_resolved_.size()) return;
		note_resolved_[cmd.note_index] = 1;
	}

	void RuntimeStore::AdvancePastMissWindow(
		const FrozenChart& chart,
		std::int32_t judge_time_ms,
		std::int32_t song_offset_ms,
		std::int32_t good_window_ms) noexcept {

		const std::size_t note_count = chart.Notes().size();
		const auto& notes = chart.Notes();
		while (next_idx_ < note_count) {
			const std::int32_t effective = notes[next_idx_].time_ms + song_offset_ms;
			if (judge_time_ms - effective <= good_window_ms) {
				break;
			}
			++next_idx_;
		}
	}
}