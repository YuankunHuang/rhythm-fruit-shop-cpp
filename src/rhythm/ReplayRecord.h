#pragma once

#include <vector>
#include <cstdint>
#include "GameplaySession.h"

namespace rfs {
	enum class ReplayEventKind {
		Tap, Update,
	};

	struct ReplayEvent {
		int lane;
		std::int32_t input_ms;
		std::int32_t song_offset_ms; // offset can be adjusted during gameplay, so it must be recored per event
		ReplayEventKind kind;
	};

	struct ReplayRecord {
		GameplaySessionConfig config;
		std::vector<ReplayEvent> events;
	};
}