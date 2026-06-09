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
		ReplayEventKind kind;
	};

	struct ReplayRecord {
		std::vector<ReplayEvent> events;
		GameplaySessionConfig config;
	};
}