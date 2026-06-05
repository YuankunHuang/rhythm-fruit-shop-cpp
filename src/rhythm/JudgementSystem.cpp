#include "JudgementSystem.h"

namespace rfs {

	TapCommandBuffer JudgementSystem::JudgeTaps(
		const FrozenChart& chart,
		std::span<const std::uint8_t> note_resolved,
		int next_idx,
		int lane,
		std::int32_t input_song_time_ms,
		std::int32_t song_offset_ms
		) const {

		TapCommandBuffer buffer{};

		int best_idx = -1;
		std::int32_t best_dt = JudgeWindows::kGood + 1;

		const auto& notes = chart.Notes();
		for (int i = next_idx; i < static_cast<int>(notes.size()); ++i) {
			if (note_resolved[i]) continue;
			if (notes[i].lane != static_cast<uint8_t>(lane)) continue;

			const std::int32_t effective = notes[i].time_ms + song_offset_ms;
			const std::int32_t dt = std::abs(effective - input_song_time_ms);
			if (dt > JudgeWindows::kGood) {
				if (effective - input_song_time_ms > JudgeWindows::kGood) break;
				continue;
			}
			if (dt < best_dt) {
				best_dt = dt;
				best_idx = i;
			}
		}

		if (best_idx < 0)
			return buffer;

		JudgeResult r =
			best_dt <= JudgeWindows::kPerfect ? JudgeResult::Perfect :
			best_dt <= JudgeWindows::kGreat ? JudgeResult::Great :
			JudgeResult::Good;
		buffer.Push(JudgeCommand{
			best_idx, r, JudgeCommand::Kind::TapHit
			});
		return buffer;
	}

	MissCommandBuffer JudgementSystem::DetectMisses(
		const FrozenChart& chart,
		std::span<const std::uint8_t> note_resolved,
		int next_idx,
		std::int32_t song_time_ms,
		std::int32_t song_offset_ms) const {

		MissCommandBuffer buffer{};

		const auto& notes = chart.Notes();

		// deal with all notes that have passed the "dead-line"
		while (next_idx < static_cast<int>(notes.size()) &&
			song_time_ms - (notes[next_idx].time_ms + song_offset_ms) > JudgeWindows::kGood) {
			if (!note_resolved[next_idx]) {
				buffer.Push(JudgeCommand{ next_idx, JudgeResult::Miss, JudgeCommand::Kind::AutoMiss });
			}
			++next_idx;
		}

		return buffer;
	}
}