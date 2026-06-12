#pragma once

#include <cstdint>
#include "GameConfig.h"

namespace rfs {
	struct PlayerSettings { // global, persistent
		bool show_debug_overlay = false;
		int32_t song_offset_ms = 0;
		int speed_idx = GameConfig::kDefaultSpeedIndex;
	};

	struct GameplayDebugState { // only gameplay
		int32_t last_judge_delta_ms = 0;
		float last_frame_duration_ms = 0.f;
	};
}