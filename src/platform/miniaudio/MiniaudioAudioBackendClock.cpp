#include "MiniaudioAudioBackendClock.h"
#include "MiniaudioAudioPlayer.h"

#include <chrono>

namespace rfs {

	MiniaudioAudioBackendClock::MiniaudioAudioBackendClock(MiniaudioAudioPlayer& player)
		: player_(player) { }

	MiniaudioAudioBackendClock::~MiniaudioAudioBackendClock() = default;

	SampleAnchor MiniaudioAudioBackendClock::Current() noexcept {
		SampleAnchor anchor{};
		anchor.host_ns = std::chrono::steady_clock::now().time_since_epoch().count();
		anchor.sample_cursor = player_.CursorInPcmFrames();
		anchor.sample_rate = player_.SampleRate();
		return anchor;
	}

	bool MiniaudioAudioBackendClock::IsArmed() const noexcept {
		return player_.IsPlaying() && player_.IsSoundLoaded();
	}
}