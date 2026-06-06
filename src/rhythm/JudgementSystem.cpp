#include "JudgementSystem.h"

namespace rfs {

	TapCommandBuffer JudgementSystem::JudgeTaps(
		const FrozenChart& chart,
		std::span<const std::uint8_t> note_resolved,
		std::size_t next_idx,
		int lane,
		std::int32_t input_song_time_ms,
		std::int32_t song_offset_ms
		) const {

		TapCommandBuffer buffer{};

		int best_idx = -1;
		std::int32_t best_dt = config_.good_window_ms + 1;

		const auto& notes = chart.Notes();
		for (auto i = next_idx; i < notes.size(); ++i) {
			if (note_resolved[i]) continue;
			if (notes[i].lane != static_cast<uint8_t>(lane)) continue;

			const std::int32_t effective = notes[i].time_ms + song_offset_ms;
			const std::int32_t dt = std::abs(effective - input_song_time_ms);
			if (dt > config_.good_window_ms) {
				if (effective - input_song_time_ms > config_.good_window_ms) break;
				continue;
			}
			if (dt < best_dt) {
				best_dt = dt;
				best_idx = static_cast<int>(i);
			}
		}

		if (best_idx < 0)
			return buffer;

		JudgeResult r =
			best_dt <= config_.perfect_window_ms ? JudgeResult::Perfect :
			best_dt <= config_.great_window_ms ? JudgeResult::Great :
			JudgeResult::Good;
		buffer.Push(JudgeCommand{
			static_cast<std::size_t>(best_idx), r, JudgeCommand::Kind::TapHit
			});
		return buffer;
	}

	MissCommandBuffer JudgementSystem::DetectMisses(
		const FrozenChart& chart,
		std::span<const std::uint8_t> note_resolved,
		std::size_t next_idx,
		std::int32_t song_time_ms,
		std::int32_t song_offset_ms) const {

		MissCommandBuffer buffer{};

		const auto& notes = chart.Notes();

		// deal with all notes that have passed the "dead-line"
		while (next_idx < notes.size() &&
			song_time_ms - (notes[next_idx].time_ms + song_offset_ms) > config_.good_window_ms) {
			if (!note_resolved[next_idx]) {
				buffer.Push(JudgeCommand{ next_idx, JudgeResult::Miss, JudgeCommand::Kind::AutoMiss });
			}
			++next_idx;
		}

		return buffer;
	}
}