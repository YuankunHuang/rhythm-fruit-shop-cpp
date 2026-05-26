#pragma once

namespace rfs {
	struct FrameContext final {
		float delta_time = 0.f;
		float song_time_ms = 0.f;
		float win_w = 1280.f;
		float win_h = 720.f;
	};
}