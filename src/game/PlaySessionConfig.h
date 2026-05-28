#pragma once

#include <cstdint>

namespace rfs {
	struct PlaySessionConfig {
		bool show_debug_overlay = false;
		int32_t input_offset_ms = 0;
		int32_t last_judge_delta_ms = 0;
		float last_frame_duration_ms = 0.f;
	};
}