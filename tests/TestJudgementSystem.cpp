#include <doctest/doctest.h>
#include "rhythm/ChartLoader.h"
#include "rhythm/JudgementSystem.h"

namespace {
	rfs::FrozenChart LoadEasyFixture() {
		rfs::ChartLoader loader;
		rfs::LoadError err;
		auto chart = loader.Load("assets/charts/test-fixture.rfs.json", "easy", err);
		REQUIRE(chart.has_value());
		return *std::move(chart);
	}

	rfs::GameplaySnapshot MakeSnapshot(std::size_t note_count) {
		rfs::GameplaySnapshot s{};
		s.next_idx = 0;
		s.note_resolved.assign(note_count, 0);
		return s;
	}
}

TEST_CASE("JudgeLanePress +50ms yields Perfect") {
	const auto& chart = LoadEasyFixture();
	auto snapshot = MakeSnapshot(chart.Notes().size());

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1050;

	auto cmd = rfs::JudgementSystem::JudgeLanePress(chart, snapshot, lane, input_ms);
	REQUIRE(cmd.has_value());
	CHECK(cmd->result == rfs::JudgeResult::Perfect);
	CHECK(cmd->note_index == 0);
	CHECK(cmd->kind == rfs::JudgeCommand::Kind::TapHit);
}

TEST_CASE("JudgeLanePress +100ms yields Great") {
	const auto& chart = LoadEasyFixture();
	auto snapshot = MakeSnapshot(chart.Notes().size());

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1100;

	auto cmd = rfs::JudgementSystem::JudgeLanePress(chart, snapshot, lane, input_ms);
	REQUIRE(cmd.has_value());
	CHECK(cmd->result == rfs::JudgeResult::Great);
	CHECK(cmd->note_index == 0);
	CHECK(cmd->kind == rfs::JudgeCommand::Kind::TapHit);
}

TEST_CASE("JudgeLanePress +151ms yields no command") {
	const auto& chart = LoadEasyFixture();
	auto snapshot = MakeSnapshot(chart.Notes().size());

	// first note predefined at time_ms = 1000, Lane = 0
	const int lane = 0;
	const std::int32_t input_ms = 1151;

	auto cmd = rfs::JudgementSystem::JudgeLanePress(chart, snapshot, lane, input_ms);
	CHECK_FALSE(cmd.has_value());
}