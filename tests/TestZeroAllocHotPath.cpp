// Zero-allocation hot-path contract.
//
// 1. "hot path performs zero heap allocations during steady-state play":
//    drives GameplaySession::HandleLaneTap + Update over a real chart and
//    asserts the steady-state loop performs no heap allocation.
// 2. "guard counts a known heap allocation": calibrates the AllocationGuard
//    itself, proving it detects a deliberate allocation (so a broken
//    instrument can't silently report zero and give false confidence).

#include <doctest/doctest.h>
#include "support/AllocationGuard.h"
#include "rhythm/ChartLoader.h"
#include "rhythm/GameplaySession.h"
#include <cstdint>
#include <cstddef>

namespace {
	rfs::FrozenChart LoadEasyFixture() {
		rfs::ChartLoader loader;
		rfs::LoadError err;
		auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
		REQUIRE(chart.has_value());
		return *std::move(chart);
	}
}

TEST_CASE("hot path performs zero heap allocations during steady-state play") {
	auto chart = LoadEasyFixture();
	rfs::GameplaySession session(std::move(chart));

	const auto& notes = session.Chart().Notes();
	REQUIRE(notes.size() >= 2); // 1 for preheat, the rest for measuring

	// 1st note: perfect tap 
	// a preheat: to trigger any lazy initialization in runtime / std, so that subsequent calls are more likely to be steady-state.
	{
		const auto& n0 = notes[0];
		const std::int32_t t0 = n0.time_ms + session.Config().song_offset_ms;
		session.HandleLaneTap(n0.lane, t0);
		session.Update(t0);
	}

	// formal measure: tap all notes perfectly, and check that no heap allocations were made in the process.
	std::size_t allocs = 0;
	{
		rfs::test::ScopedAllocGuard guard(false);
		for (std::size_t i = 1; i < notes.size(); ++i) {
			const auto& n = notes[i];
			const std::int32_t t = n.time_ms + session.Config().song_offset_ms;
			session.HandleLaneTap(n.lane, t);
			session.Update(t);
		}
		allocs = guard.allocations(); // read the value within the guard's scope, to ensure it counts all allocations made during the loop
	}

	CHECK(allocs == 0); // evaluate outside the guard's scope, to avoid any potential allocations from the guard's destructor
}

TEST_CASE("guard counts a known heap allocation") {
	std::size_t allocs = 0;
	int* p = nullptr;
	{
		rfs::test::ScopedAllocGuard guard(false);
		p = new int(42); // deliberate allocation
		allocs = guard.allocations();
	}

	CHECK(p != nullptr);
	CHECK(allocs == 1); // the guard should have counted the single allocation made above)
	delete p;
}