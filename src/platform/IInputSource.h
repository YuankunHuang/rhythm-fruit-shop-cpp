#pragma once

#include "InputEvent.h"
#include <span>

namespace rfs {
	class IInputSource {
	public:
		virtual ~IInputSource() = default;
		virtual std::span<InputEvent> Poll(HostNanos poll_enter_ns) noexcept = 0;
	};
}