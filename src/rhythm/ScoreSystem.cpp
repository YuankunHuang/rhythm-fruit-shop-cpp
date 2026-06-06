#include "ScoreSystem.h"

namespace rfs {

	namespace {

		// base score - integers
		inline constexpr std::int32_t kBasePerfect = 1000;
		inline constexpr std::int32_t kBaseGreat = 700;
		inline constexpr std::int32_t kBaseGood = 300;

		// Q16 fixed-point multiplier for combo bonus
		inline constexpr std::int32_t kOneQ16 = 1 << 16;
		inline constexpr std::int32_t kComboStepQ16 = 328; // 1 combo -> +0.5%
		inline constexpr std::int32_t kComboCap = 100; // max combo considered for bonus

		inline std::int32_t BaseScore(JudgeResult result) noexcept {
			switch (result) {
			case JudgeResult::Perfect: return kBasePerfect;
			case JudgeResult::Great: return kBaseGreat;
			case JudgeResult::Good: return kBaseGood;
			default: return 0; // miss or invalid
			}
		}

		inline std::int32_t EarnScore(JudgeResult result, int32_t combo) noexcept {
			const int32_t base = BaseScore(result);
			const int32_t capped = combo < kComboCap ? combo : kComboCap;
			const int32_t multiplier_q16 = kOneQ16 + capped * kComboStepQ16;
			return static_cast<int32_t>((static_cast<int64_t>(base) * multiplier_q16) >> 16); // int64 to prevent overflow during multiplication
		}
	}

	void ScoreSystem::Apply(const JudgeCommand& cmd) noexcept {

		switch (cmd.kind) {
		case JudgeCommand::Kind::AutoMiss:
			combo_ = 0;
			++cnt_miss_;
			break;
		case JudgeCommand::Kind::TapHit:
			++combo_;
			switch (cmd.result) {
			case JudgeResult::Perfect:
				++cnt_perfect_;
				break;
			case JudgeResult::Great:
				++cnt_great_;
				break;
			case JudgeResult::Good:
				++cnt_good_;
				break;
			}
			break;
		}

		if (combo_ > max_combo_) {
			max_combo_ = combo_;
		}
		score_ += EarnScore(cmd.result, combo_);
	}

	void ScoreSystem::Reset() noexcept {
		score_ = 0;
		combo_ = 0;
		max_combo_ = 0;
		cnt_perfect_ = 0;
		cnt_great_ = 0;
		cnt_good_ = 0;
		cnt_miss_ = 0;
	}
}