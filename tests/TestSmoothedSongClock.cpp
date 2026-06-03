#include <doctest/doctest.h>
#include "rhythm/SmoothedSongClock.h"

TEST_CASE("SmoothedSongClock works through freeze") {
	rfs::SmoothedSongClock clock{};
	rfs::SampleAnchor anchor{};
	anchor.sample_rate = 48000;
	anchor.sample_cursor = 48000;
	anchor.host_ns = 1'000'000'000;
	clock.Tick(anchor, anchor.host_ns);

	const rfs::HostNanos host_ns_at_pause = anchor.host_ns + 500'000'000;
	const float t0 = clock.NowMs(host_ns_at_pause);

	clock.SetFrozen(t0, host_ns_at_pause);

	const rfs::HostNanos host_ns_after_pause = host_ns_at_pause + 3'000'000'000;
	CHECK(t0 == clock.NowMs(host_ns_after_pause)); // when frozen, clock.NowMs stays unchanged

	clock.ClearFrozen(host_ns_after_pause);

	CHECK(clock.NowMs(host_ns_after_pause) == t0); // moment of defreezing, no gap
	CHECK(clock.NowMs(host_ns_after_pause + 16'000'000) == t0 + 16.f);
}