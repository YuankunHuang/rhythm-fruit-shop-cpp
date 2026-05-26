#pragma once

#include <cstdint>

namespace rfs {
	using HostNanos = std::int64_t;
	using Milliseconds = std::int32_t;

	enum class InputAction : std::uint8_t {
		Lane0, Lane1, Lane2, Lane3,
		Pause, Restart, ToggleDebug, CycleCalibration,
		NavUp, NavDown, NavLeft, NavRight,
	};

	struct InputEvent final {
		InputAction action = InputAction::Lane0;
		bool pressed = false;
		std::uint8_t _pad0 = 0;
		std::uint8_t _pad1 = 0;
		HostNanos event_host_ns = 0;
		Milliseconds event_song_time_ms = 0;
	};
}