#pragma once

#include "InputEvent.h"
#include <span>

namespace rfs {
	class IInputSource {
	public:
		virtual ~IInputSource() = default;
		virtual std::span<const InputEvent> Poll(HostNanos pollEnterHostNs) noexcept = 0;
	};
}