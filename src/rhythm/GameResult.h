#pragma once

namespace rfs {
	struct GameResult {
		int score = 0;
		int combo = 0; // highest
		int perfect = 0;
		int great = 0;
		int good = 0;
		int miss = 0;

		bool operator==(const GameResult& other) const noexcept {
			return score == other.score
				&& combo == other.combo
				&& perfect == other.perfect
				&& great == other.great
				&& good == other.good
				&& miss == other.miss;
		}
	};
}