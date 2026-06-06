#pragma once

#include <cstddef>

namespace rfs {
	enum class JudgeResult {
		Perfect, Great, Good, Miss
	};

	struct JudgeCommand {
		std::size_t note_index = 0;
		JudgeResult result = JudgeResult::Miss;
		enum class Kind { AutoMiss, TapHit } kind = Kind::AutoMiss;
	};
}