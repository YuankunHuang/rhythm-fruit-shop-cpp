#pragma once

#include "GameResult.h"
#include "../rhythm/JudgementSystem.h"
#include <cstdint>

namespace rfs {

	namespace Scoring {

		// base score - integers
		inline constexpr int32_t kBasePerfect = 1000;
		inline constexpr int32_t kBaseGreat = 700;
		inline constexpr int32_t kBaseGood = 300;

		// Q16 fixed-point multiplier for combo bonus
		inline constexpr int32_t kOneQ16 = 1 << 16;
		inline constexpr int32_t kComboStepQ16 = 328; // 1 combo -> +0.5%
		inline constexpr int32_t kComboCap = 100; // max combo considered for bonus

		inline int32_t BaseScore(JudgeResult result) noexcept {
			switch (result) {
			case JudgeResult::Perfect: return kBasePerfect;
			case JudgeResult::Great: return kBaseGreat;
			case JudgeResult::Good: return kBaseGood;
			default: return 0; // miss or invalid
			}
		}

		inline int32_t EarnScore(JudgeResult result, int32_t combo) noexcept {
			const int32_t base = BaseScore(result);
			const int32_t capped = combo < kComboCap ? combo : kComboCap;
			const int32_t multiplier_q16 = kOneQ16 + capped * kComboStepQ16;
			return static_cast<int32_t>((static_cast<int64_t>(base) * multiplier_q16) >> 16); // int64 to prevent overflow during multiplication
		}
	}

	namespace Grading {
		inline float Accuracy(const GameResult& r) {
			int total = r.perfect + r.great + r.good + r.miss;
			if (total == 0) {
				return 0.f; // WTF
			}
			float acc = static_cast<float>(r.perfect * 300 + r.great * 200 + r.good * 100)
				/ static_cast<float>(total * 300);
			return acc;
		}

		inline const char* CalcGrade(const GameResult& r) {
			float acc = Accuracy(r);
			if (acc >= 0.95f) return "S";
			if (acc >= 0.85f) return "A";
			if (acc >= 0.70f) return "B";
			if (acc >= 0.60f) return "C";
			return "D";
		}
	}
}