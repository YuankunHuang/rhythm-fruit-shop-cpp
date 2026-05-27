#include "SmoothedSongClock.h"
#include <cmath>
#include <algorithm>

namespace rfs {

	namespace {
		constexpr float kDeadBandMs = 1.f;
		constexpr float kSoftReanchorMs = 8.f;
		constexpr float kEmaAlpha = 0.25f;
	}

	void SmoothedSongClock::Reset() {
		armed_ = false;
		frozen_ = false;
		frozen_ms_ = 0.f;
		anchor_song_ms_ = 0.f;
		anchor_host_ns_ = 0;
		sample_rate_ = 48000;
	}

	float SmoothedSongClock::SampleToMs(SampleAnchor anchor) noexcept {
		if (anchor.sample_rate <= 0) {
			return 0.f;
		}
		return static_cast<float>(anchor.sample_cursor)
			/ static_cast<float>(anchor.sample_rate)
			* 1000.f;
	}

	void SmoothedSongClock::Tick(SampleAnchor anchor, [[maybe_unused]] HostNanos host_now_ns) {
		if (frozen_) {
			return;
		}
		if (anchor.sample_rate <= 0 || anchor.sample_cursor == 0) {
			return;
		}

		const float observed_ms = SampleToMs(anchor);
		sample_rate_ = anchor.sample_rate;

		if (!armed_) {
			armed_ = true;
			anchor_song_ms_ = observed_ms;
			anchor_host_ns_ = anchor.host_ns;
			return;
		}

		const float elapsed_ms = static_cast<float>(anchor.host_ns - anchor_host_ns_) / 1'000'000.f;
		const float predicted_ms = anchor_song_ms_ + elapsed_ms;
		const float delta_ms = observed_ms - predicted_ms;
		const float abs_delta = std::abs(delta_ms);

		if (abs_delta <= kDeadBandMs) {
			return;
		}

		if (abs_delta <= kSoftReanchorMs) {
			anchor_song_ms_ += delta_ms * kEmaAlpha;
			anchor_host_ns_ = anchor.host_ns;
		}
		else {
			anchor_song_ms_ = observed_ms;
			anchor_host_ns_ = anchor.host_ns;
		}
	}

	float SmoothedSongClock::NowMs(HostNanos host_now_ns) const noexcept {
		if (frozen_) {
			return frozen_ms_;
		}
		if (!armed_) {
			return 0.f;
		}
		const float elapsed_ms = static_cast<float>(host_now_ns - anchor_host_ns_) / 1'000'000.f;
		return anchor_song_ms_ + elapsed_ms;
	}

	std::int32_t SmoothedSongClock::HostNsToSongTimeMs(HostNanos event_host_ns) const noexcept {
		return static_cast<std::int32_t>(NowMs(event_host_ns));
	}

	bool SmoothedSongClock::IsArmed() const noexcept { return armed_; }

	void SmoothedSongClock::SetFrozen(float song_time_ms, HostNanos host_ns_now) noexcept {
		frozen_ms_ = song_time_ms;

		// re-seed, to ensure no frame-gap
		anchor_song_ms_ = song_time_ms;
		anchor_host_ns_ = host_ns_now;

		frozen_ = true;
	}

	void SmoothedSongClock::ClearFrozen(HostNanos host_ns_after_pause) noexcept {
		if (frozen_) {
			anchor_song_ms_ = frozen_ms_;
			anchor_host_ns_ = host_ns_after_pause; // re-seed to avoid "same-frame snap"
		}

		frozen_ = false;
	}

}
