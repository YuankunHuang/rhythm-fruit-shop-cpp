#pragma once

#include "../platform/SampleAnchor.h"
#include <cstdint>

namespace rfs {

	// L1 song clock: sample anchor + host-clock interpolation + EMA reanchor.
	class SmoothedSongClock {
	public:
		void Reset();
		void Tick(SampleAnchor anchor, HostNanos host_now_ns);
		float NowMs(HostNanos host_now_ns) const noexcept;
		std::int32_t HostNsToSongTimeMs(HostNanos event_host_ns) const noexcept;
		bool IsArmed() const noexcept;

		void SetFrozen(float song_time_ms, HostNanos host_ns_now) noexcept;
		void ClearFrozen(HostNanos host_ns_after_pause) noexcept;
		bool IsFrozen() const noexcept { return frozen_; }

	private:
		static float SampleToMs(SampleAnchor anchor) noexcept;

		bool armed_ = false;
		bool frozen_ = false;
		float frozen_ms_ = 0.f;

		float anchor_song_ms_ = 0.f;
		HostNanos anchor_host_ns_ = 0;
		std::int32_t sample_rate_ = 48000;
	};

}
