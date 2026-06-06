#pragma once

#include "../rhythm/GameResult.h"
#include <cstdint>

namespace rfs {

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