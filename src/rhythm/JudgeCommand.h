#pragma once

namespace rfs {
	enum class JudgeResult {
		Perfect, Great, Good, Miss
	};

	struct JudgeCommand {
		int note_index = -1;
		JudgeResult result = JudgeResult::Miss;
		enum class Kind { AutoMiss, TapHit } kind = Kind::AutoMiss;
	};
}