#include "SmoothedSongClock.h"

namespace rfs {

	void SmoothedSongClock::Reset() {
		armed_ = false;
		now_ms_ = 0.f;
	}

	void SmoothedSongClock::Tick(SampleAnchor anchor) {
		if (anchor.sample_rate <= 0 || anchor.sample_cursor == 0) {
			return;
		}
		armed_ = true;
		now_ms_ = static_cast<float>(anchor.sample_cursor)
		        / static_cast<float>(anchor.sample_rate)
		        * 1000.f;
	}

	float SmoothedSongClock::NowMs() const noexcept { return now_ms_; }
	bool  SmoothedSongClock::IsArmed() const noexcept { return armed_; }

}
