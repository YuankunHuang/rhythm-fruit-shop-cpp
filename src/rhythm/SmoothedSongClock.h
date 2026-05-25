#pragma once

#include "../platform/SampleAnchor.h"

namespace rfs {

	// Converts raw PCM sample cursor from the audio backend into song time (ms).
	// Call Tick() once per frame with the latest SampleAnchor from IAudioBackendClock.
	class SmoothedSongClock {
	public:
		void Reset();
		void Tick(SampleAnchor anchor);
		float NowMs() const noexcept;
		bool IsArmed() const noexcept;

	private:
		bool armed_ = false;
		float now_ms_ = 0.f;
	};

}
