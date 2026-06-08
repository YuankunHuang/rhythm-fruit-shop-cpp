#pragma once

#include "FrozenChart.h"
#include <cstdint>
#include "JudgementSystem.h"
#include "GameResult.h"
#include "RuntimeStore.h"
#include "ScoreSystem.h"
#include <optional>
#include "JudgeCommandBuffer.h"

namespace rfs {
	struct GameplaySessionConfig {
		std::int32_t song_offset_ms = 0;
		std::int32_t song_end_delay_ms = 2000;
		JudgementConfig judgement{};
	};

	class GameplaySession final {
	public:
		explicit GameplaySession(FrozenChart chart, GameplaySessionConfig config = {});

		std::optional<TapCommandBuffer> HandleLaneTap(int lane, std::int32_t input_ms);
		MissCommandBuffer Update(std::int32_t song_time_ms);
		bool IsFinished(std::int32_t song_time_ms) const noexcept;
		GameResult Summary() const noexcept;

		const FrozenChart& Chart() const noexcept { return chart_; }
		const RuntimeStore& Store() const noexcept { return store_; }
		const ScoreSystem& Score() const noexcept { return score_; }
		const GameplaySessionConfig& Config() const noexcept { return config_; }
		std::int32_t GameplayEndMs() const noexcept { return gameplay_end_ms_; }
		std::int32_t ChartEndMs() const noexcept { return chart_end_ms_; }

	private:
		void CommitCommand(const JudgeCommand& cmd) noexcept;

		FrozenChart chart_;
		GameplaySessionConfig config_;
		RuntimeStore store_;
		JudgementSystem judge_{};
		ScoreSystem score_{};
		std::int32_t gameplay_end_ms_ = 0;
		std::int32_t chart_end_ms_ = 0;
	};
}
