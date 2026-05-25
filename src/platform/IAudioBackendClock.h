#pragma once

#include "SampleAnchor.h"

namespace rfs {
	class IAudioBackendClock {
	public:
		virtual ~IAudioBackendClock() = default;

		// sampling takes place every frame
		virtual SampleAnchor Current() noexcept = 0;
		virtual bool IsArmed() const noexcept = 0; // whether the clock is ready to provide valid timing information (e.g. after audio playback has started)
	};
}