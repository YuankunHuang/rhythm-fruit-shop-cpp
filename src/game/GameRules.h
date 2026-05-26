#pragma once

#include "GameResult.h"
#include <cstdint>

namespace rfs {
	enum class JudgeResult {
		Perfect, Great, Good, Miss
	};

	namespace JudgeWindows {
		constexpr int32_t kPerfect = 50; // +-50ms
		constexpr int32_t kGreat = 100;
		constexpr int32_t kGood = 150;
		// greater diff -> miss
	}

	namespace Scoring {
		inline int32_t EarnScore(JudgeResult result, int32_t combo) {
			int base = 0;
			switch (result) {
			case JudgeResult::Perfect: base = 300; break;
			case JudgeResult::Great: base = 200; break;
			case JudgeResult::Good: base = 100; break;
			}

			float multiplier = 1.0f;
			if (combo >= 100) multiplier = 2.0f;
			else if (combo >= 50) multiplier = 1.5f;
			else if (combo >= 20) multiplier = 1.2f;

			return static_cast<int>(base * multiplier);
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