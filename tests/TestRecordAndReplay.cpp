#include <doctest/doctest.h>
#include "rhythm/FrozenChart.h"
#include "rhythm/GameplaySession.h"
#include "rhythm/ReplayRecord.h"
#include "rhythm/RecordingSession.h"
#include "rhythm/ReplayHeadless.h"
#include "rhythm/ChartLoader.h"

namespace {
	rfs::FrozenChart LoadEasyFixture() {
		rfs::ChartLoader loader;
		rfs::LoadError err;
		auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
		REQUIRE(chart.has_value());
		return *std::move(chart);
	}
}

TEST_CASE("Positive: Record and replay a gameplay session") {
	using namespace rfs;
	// Create a simple chart with 3 notes.
	FrozenChart chart = LoadEasyFixture();
	// Simulate a recording session with some taps and updates.
	ReplayRecord record{};
	GameResult live{};
	{
		RecordingSession session(chart, GameplaySessionConfig{ .song_offset_ms = -50 }, record);
		session.HandleLaneTap(0, 950);   // Early tap for first note (Perfect)
		session.HandleLaneTap(1, 1550);  // Late tap for second note (Great)
		session.HandleLaneTap(2, 2100);  // Late tap for third note (Good)
		session.Update(2500); // Process any remaining misses
		live = session.Gameplay().Summary();
		// end of session!
	}

	// Now replay the recorded session headlessly and verify the result.
	GameResult result = ReplayHeadless(chart, record);
	CHECK(result == live);
}

TEST_CASE("Negative: Replay outcome depends on record.config (identity)") {
	using namespace rfs;
	FrozenChart chart = LoadEasyFixture();
	ReplayRecord record{};
	{
		RecordingSession session(chart, GameplaySessionConfig{ .song_offset_ms = -50 }, record);
		session.HandleLaneTap(0, 950);
		session.HandleLaneTap(1, 2030);
		session.HandleLaneTap(2, 3080);
		session.Update(4200);
	}
	const GameResult faithful = ReplayHeadless(chart, record);
	ReplayRecord tampered = record;
	tampered.config.song_offset_ms -= 100000; // negative! This should cause all taps to miss. Otherwise, notes remain unresolved, a bit confusing for assertions.
	const GameResult diverged = ReplayHeadless(chart, tampered);
	CHECK(diverged != faithful);
	CHECK(diverged.perfect == 0);
	CHECK(diverged.miss == static_cast<int>(chart.Notes().size()));
}