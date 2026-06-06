#include <doctest/doctest.h>
#include "rhythm/ChartLoader.h"
#include "rhythm/GameplaySession.h"

namespace {
	rfs::FrozenChart LoadEasyFixture() {
		rfs::ChartLoader loader;
		rfs::LoadError err;
		auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
		REQUIRE(chart.has_value());
		return *std::move(chart);
	}
}

TEST_CASE("GameplaySession smoke - tap + update") {
	auto chart = LoadEasyFixture();
	rfs::GameplaySession session{ std::move(chart) };

	const auto taps = session.HandleLanePress(0, 1050);
	REQUIRE(taps.has_value());
	CHECK(taps->count == 1);
	CHECK(session.Score().Combo() == 1);

	session.Update(2000);
	CHECK_FALSE(session.IsFinished(2000));
	CHECK(session.Store().NextIdx() < session.Chart().Notes().size());
}

TEST_CASE("GameplaySession IsFinished after chart end") {
	auto chart = LoadEasyFixture();
	rfs::GameplaySession session{ std::move(chart) };

	const std::int32_t end_ms = session.ChartEndMs();
	for (std::int32_t t = 0; t <= end_ms; t += 100) {
		session.Update(t);
	}
	CHECK(session.IsFinished(end_ms));
}
