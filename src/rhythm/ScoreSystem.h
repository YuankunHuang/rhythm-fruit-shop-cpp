#pragma once

#include <cstdint>
#include "JudgeCommand.h"
#include "GameResult.h"

namespace rfs {

	class ScoreSystem final {
	public:
		std::int32_t Score() const noexcept { return score_; }
		std::int32_t Combo() const noexcept { return combo_; }
		std::int32_t MaxCombo() const noexcept { return max_combo_; }
		GameResult BuildResult() const noexcept {
			return GameResult{
				.score = score_,
				.combo = max_combo_,
				.perfect = cnt_perfect_,
				.great = cnt_great_,
				.good = cnt_good_,
				.miss = cnt_miss_,
			};
		}
		void Apply(const JudgeCommand& cmd) noexcept;
		void Reset() noexcept;

	private:
		std::int32_t score_ = 0;
		std::int32_t combo_ = 0;
		std::int32_t max_combo_ = 0;
		std::int32_t cnt_perfect_ = 0;
		std::int32_t cnt_great_ = 0;
		std::int32_t cnt_good_ = 0;
		std::int32_t cnt_miss_ = 0;
	};
}