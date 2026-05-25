#pragma once

#include "../IAudioBackendClock.h"
#include <memory>

namespace rfs {
	class MiniaudioAudioPlayer; // forward declaration to avoid circular dependency, we only need a ref/ptr to it
	class MiniaudioAudioBackendClock final : public IAudioBackendClock {
	public:
		explicit MiniaudioAudioBackendClock(MiniaudioAudioPlayer& player);
		~MiniaudioAudioBackendClock() override;
		SampleAnchor Current() noexcept override;
		bool IsArmed() const noexcept override;

	private:
		MiniaudioAudioPlayer& player_; // reference to the audio player, used to query playback state and timing information
	};
}