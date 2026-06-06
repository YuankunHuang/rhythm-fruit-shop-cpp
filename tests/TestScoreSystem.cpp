#include <doctest/doctest.h>
#include "rhythm/ScoreSystem.h"
#include "rhythm/JudgeCommand.h"

TEST_CASE("ScoreSystem - Basic Operations") {
	rfs::ScoreSystem score{};
	score.Apply({
		.note_index = 0,
		.result = rfs::JudgeResult::Perfect,
		.kind = rfs::JudgeCommand::Kind::TapHit
		});
	CHECK(score.Score() > 0);
	CHECK(score.Combo() == 1);
	CHECK(score.MaxCombo() == 1);
	auto result = score.BuildResult();
	CHECK(result.perfect == 1);
	CHECK(result.great == 0);
	CHECK(result.good == 0);
	CHECK(result.miss == 0);

	score.Apply({
	.note_index = 0,
	.result = rfs::JudgeResult::Miss,
	.kind = rfs::JudgeCommand::Kind::AutoMiss
		});
	CHECK(score.Score() > 0);
	CHECK(score.Combo() == 0);
	CHECK(score.MaxCombo() == 1);
	result = score.BuildResult();
	CHECK(result.perfect == 1);
	CHECK(result.great == 0);
	CHECK(result.good == 0);
	CHECK(result.miss == 1);
}