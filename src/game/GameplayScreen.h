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
#include <cstdint>

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

		// Hit fx
		struct HitSpark {
			float cx = 0.f;
			float cy = 0.f;
			float age_ms = 0.f;
			std::uint32_t color = 0;
			int lane = 0;
		};
		std::vector<HitSpark> hit_sparks_;
		int last_spark_idx_ = -1;
		void SpawnHitFx(int lane, JudgeResult result);
		void UpdateHitFx(float delta_sec);
		void RenderHitFx();

		uint8_t LaneCount() const { return GameConfig::kLaneCount; }
		void ApplyCommand(const JudgeCommand& cmd);

		GameContext ctx_;
		FrozenChart chart_;
		std::string cover_path_;
		float song_time_ms_ = 0.f;
		GameLayout  layout_{};
		GameConfig::UiLayout ui_{};

		float chart_end_ms_ = 1.f;
		float gameplay_end_ms_ = 0.f;
		float progress_time_ms_ = 0.f;
		bool  is_in_outro_ = false;
		float audio_volume_ = 1.f;
		bool  result_pushed_ = false;

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

		// lead in
		bool is_in_lead_in_ = false;
		float lead_in_ms_ = 0.f;

		GameResult BuildResult() const;

		GameplaySnapshot snapshot_;
	};
}
