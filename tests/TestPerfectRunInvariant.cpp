#include <doctest/doctest.h>
#include "rhythm/ChartLoader.h"
#include "rhythm/GameplaySession.h"
#include <cstdint>

namespace {
	rfs::FrozenChart LoadEasyFixture() {
		rfs::ChartLoader loader;
		rfs::LoadError err;
		auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
		REQUIRE(chart.has_value());
		return *std::move(chart);
	}

	// Independent oracle: mirrors ScoreSystem's per-note Q16 combo bonus for an
	// all-Perfect run. Must replicate the engine's per-note truncation and order
	// (combo is incremented BEFORE scoring), so we accumulate note-by-note rather
	// than using a closed form.
	std::int32_t ExpectedMaxScore(const rfs::FrozenChart& chart) {
		constexpr std::int32_t kBasePerfect = 1000;
		constexpr std::int32_t kOneQ16 = 1 << 16;
		constexpr std::int32_t kComboStepQ16 = 328;
		constexpr std::int32_t kComboCap = 100;

		std::int32_t total = 0;
		std::int32_t combo = 0;
		for (std::size_t i = 0; i < chart.Notes().size(); ++i) {
			++combo; // ScoreSystem increments before earning
			const std::int32_t capped = combo < kComboCap ? combo : kComboCap;
			const std::int32_t mult_q16 = kOneQ16 + capped * kComboStepQ16;
			total += static_cast<std::int32_t>(
				(static_cast<std::int64_t>(kBasePerfect) * mult_q16) >> 16);
		}
		return total;
	}
}

// Drive the headless session with flawless input: tap every note at its exact
// effective time, in chart (time) order. Asserts the macro determinism
// invariant: all-Perfect, zero misses, full combo, and theoretical max score.
TEST_CASE("PerfectRunInvariant") {
	auto chart = LoadEasyFixture();
	rfs::GameplaySession session{ std::move(chart) };

	const std::size_t note_count = session.Chart().Notes().size();
	REQUIRE(note_count > 0);

	int resolved = 0;
	for (const auto& note : session.Chart().Notes()) {
		const std::int32_t input_ms = note.time_ms + session.Config().song_offset_ms;
		auto taps = session.HandleLaneTap(note.lane, input_ms);
		REQUIRE(taps.has_value());
		CHECK(taps->count == 1); // exactly one note resolved per perfect tap

		for (const auto& cmd : taps->Span()) {
			CHECK(cmd.result == rfs::JudgeResult::Perfect);
			CHECK(cmd.kind == rfs::JudgeCommand::Kind::TapHit);
			CHECK(cmd.note_index < note_count);
			++resolved;

			const auto result = session.Summary();
			CHECK(result.perfect == resolved);
			CHECK(result.combo == resolved); // running max combo == notes hit so far
			CHECK(result.great == 0);
			CHECK(result.good == 0);
			CHECK(result.miss == 0);
		}

		// Advancing time to the just-tapped note must not manufacture misses.
		session.Update(input_ms);
	}

	const auto result_final = session.Summary();
	CHECK(result_final.perfect == static_cast<int>(note_count));
	CHECK(result_final.combo == static_cast<int>(note_count));
	CHECK(result_final.great == 0);
	CHECK(result_final.good == 0);
	CHECK(result_final.miss == 0);
	CHECK(session.Score().Score() == ExpectedMaxScore(session.Chart()));
}