#pragma once

#include "../app/IScreen.h"
#include "../app/FrameContext.h"
#include "../rhythm/FrozenChart.h"
#include "../rhythm/JudgementSystem.h"
#include "GameContext.h"
#include "GameLayout.h"
#include "GameConfig.h"
#include "GameResult.h"
#include "GameRules.h"
#include <string>
#include <vector>

namespace rfs {
	class GameplayScreen : public IScreen {
	public:
		GameplayScreen(GameContext ctx, FrozenChart chart, std::string cover_path);

		void OnEnter() override;
		void OnPause() override;
		void OnResume() override;
		void OnExit() override;

		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		uint8_t LaneCount() const { return GameConfig::kLaneCount; }
		void ApplyCommand(const JudgeCommand& cmd);

		GameContext ctx_;
		FrozenChart chart_;
		std::string cover_path_;
		float song_time_ms_ = 0.f;
		GameLayout  layout_{};
		GameConfig::UiLayout ui_{};

		// judgement display
		JudgeResult last_judge_{};
		float judge_display_ms_ = 0.f;

		// scoring
		int score_ = 0;
		int combo_ = 0;
		int max_combo_ = 0;
		int cnt_perfect_ = 0;
		int cnt_great_ = 0;
		int cnt_good_ = 0;
		int cnt_miss_ = 0;

		// pause
		bool paused_ = false;

		// song ending
		bool song_ending_ = false;
		float end_timer_ms_ = 0.f;
		bool result_pushed_ = false;

		GameResult BuildResult() const;

		GameplaySnapshot snapshot_;
	};
}
