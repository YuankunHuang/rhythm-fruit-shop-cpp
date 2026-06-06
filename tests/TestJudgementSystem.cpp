#include <doctest/doctest.h>
#include "rhythm/ChartLoader.h"
#include "rhythm/JudgementSystem.h"
#include "rhythm/RuntimeStore.h"

namespace {
	rfs::FrozenChart LoadEasyFixture() {
		rfs::ChartLoader loader;
		rfs::LoadError err;
		auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
		REQUIRE(chart.has_value());
		return *std::move(chart);
	}
}

TEST_CASE("JudgeLanePress +50ms yields Perfect") {
	const rfs::FrozenChart& chart = LoadEasyFixture();
	const rfs::RuntimeStore store{chart.Notes().size()};

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1050;

	rfs::JudgementSystem judge;
	const rfs::TapCommandBuffer& taps = judge.JudgeTaps(chart, store.NoteResolved(), store.NextIdx(), lane, input_ms);
	CHECK(taps.count == 1);
	for (const auto& cmd : taps.Span()) {
		CHECK(cmd.result == rfs::JudgeResult::Perfect);
		CHECK(cmd.note_index == 0);
		CHECK(cmd.kind == rfs::JudgeCommand::Kind::TapHit);
	}
}

TEST_CASE("JudgeLanePress +100ms yields Great") {
	const rfs::FrozenChart& chart = LoadEasyFixture();
	const rfs::RuntimeStore store{ chart.Notes().size() };

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1100;

	rfs::JudgementSystem judge;
	const rfs::TapCommandBuffer& taps = judge.JudgeTaps(chart, store.NoteResolved(), store.NextIdx(), lane, input_ms);
	CHECK(taps.count == 1);
	for (const auto& cmd : taps.Span()) {
		CHECK(cmd.result == rfs::JudgeResult::Great);
		CHECK(cmd.note_index == 0);
		CHECK(cmd.kind == rfs::JudgeCommand::Kind::TapHit);
	}
}

TEST_CASE("JudgeLanePress +151ms yields no command") {
	const rfs::FrozenChart& chart = LoadEasyFixture();
	const rfs::RuntimeStore store{ chart.Notes().size() };

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1151;

	rfs::JudgementSystem judge;
	const rfs::TapCommandBuffer& taps = judge.JudgeTaps(chart, store.NoteResolved(), store.NextIdx(), lane, input_ms);
	CHECK(taps.count == 0);
}

TEST_CASE("JudgeTaps with +50 offset: input at effective time yields Perfect") {

	const rfs::FrozenChart& chart = LoadEasyFixture();
	const rfs::RuntimeStore store{ chart.Notes().size() };

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1000;
	const std::int32_t offset_ms = 50;

	rfs::JudgementSystem judge;
	const rfs::TapCommandBuffer& taps = judge.JudgeTaps(chart, store.NoteResolved(), store.NextIdx(), lane, input_ms, offset_ms);
	CHECK(taps.count == 1);
	for (const auto& cmd : taps.Span()) {
		CHECK(cmd.result == rfs::JudgeResult::Perfect);
		CHECK(cmd.note_index == 0);
		CHECK(cmd.kind == rfs::JudgeCommand::Kind::TapHit);
	}
}
TEST_CASE("DetectMisses with +50 offset: no miss before effective+Good") {
	const rfs::FrozenChart& chart = LoadEasyFixture();
	const rfs::RuntimeStore store{ chart.Notes().size() };

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1201;
	const std::int32_t offset_ms = 50;

	rfs::JudgementSystem judge;
	const rfs::TapCommandBuffer& taps = judge.JudgeTaps(chart, store.NoteResolved(), store.NextIdx(), lane, input_ms, offset_ms);
	CHECK(taps.count == 0);
	auto misses = judge.DetectMisses(chart, store.NoteResolved(), store.NextIdx(), input_ms, offset_ms);
	CHECK(misses.count == 1);
	for (const auto& cmd : misses.Span()) {
		CHECK(cmd.result == rfs::JudgeResult::Miss);
		CHECK(cmd.note_index == 0);
		CHECK(cmd.kind == rfs::JudgeCommand::Kind::AutoMiss);
	}
}