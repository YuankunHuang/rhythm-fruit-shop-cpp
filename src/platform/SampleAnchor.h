#pragma once

#include "InputEvent.h"
#include <cstdint>

namespace rfs {

	using SampleIndex = std::uint64_t;

	struct SampleAnchor final {
		SampleIndex sample_cursor = 0;
		HostNanos host_ns = 0;
		std::int32_t sample_rate = 48000;
	};
}