#include "JudgementSystem.h"

namespace rfs {
	std::vector<JudgeCommand> JudgementSystem::AdvanceAutoMisses(
		const FrozenChart& chart,
		GameplaySnapshot& snapshot,
		float song_time_ms
	) {
		std::vector<JudgeCommand> out;
		const auto& notes = chart.Notes();

		// deal with all notes that have passed the "dead-line"
		while (snapshot.next_idx < static_cast<int>(notes.size()) &&
			song_time_ms - notes[snapshot.next_idx].time_ms > JudgeWindows::kGood) {
			if (!snapshot.note_resolved[snapshot.next_idx]) {
				snapshot.note_resolved[snapshot.next_idx] = 1;
				out.push_back({ snapshot.next_idx, JudgeResult::Miss, JudgeCommand::Kind::AutoMiss });
			}
			++snapshot.next_idx;
		}

		return out;
	}

	std::optional<JudgeCommand> JudgementSystem::JudgeLanePress(
		const FrozenChart& chart,
		GameplaySnapshot& snapshot,
		int lane,
		std::int32_t input_song_time_ms
	) {
		int best_idx = -1;
		std::int32_t best_dt = JudgeWindows::kGood + 1;

		const auto& notes = chart.Notes();
		for (int i = snapshot.next_idx; i < static_cast<int>(notes.size()); ++i) {
			if (snapshot.note_resolved[i]) continue;
			if (notes[i].lane != static_cast<uint8_t>(lane)) continue;

			std::int32_t dt = std::abs(notes[i].time_ms - input_song_time_ms);
			if (dt > JudgeWindows::kGood) {
				if (notes[i].time_ms - input_song_time_ms > JudgeWindows::kGood) break;
				continue;
			}
			if (dt < best_dt) { 
				best_dt = dt; 
				best_idx = i;
			}
		}

		if (best_idx < 0) 
			return std::nullopt;

		snapshot.note_resolved[best_idx] = 1;
		JudgeResult r =
			best_dt <= JudgeWindows::kPerfect ? JudgeResult::Perfect :
			best_dt <= JudgeWindows::kGreat ? JudgeResult::Great :
			JudgeResult::Good;
		return JudgeCommand{
			best_idx, r, JudgeCommand::Kind::TapHit
		};
	}
}