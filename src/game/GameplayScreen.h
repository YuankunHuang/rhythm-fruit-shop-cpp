#pragma once

#include "../app/IScreen.h"
#include "../app/FrameContext.h"
#include "../rhythm/FrozenChart.h"
#include "../rhythm/GameplaySession.h"
#include "../rhythm/JudgeCommand.h"
#include "GameContext.h"
#include "GameLayout.h"
#include "GameConfig.h"
#include <string>
#include <vector>
#include <cstdint>

namespace rfs {

	class GameplayScreen : public IScreen {
	public:
		explicit GameplayScreen(const GameContext& ctx, FrozenChart chart, std::string cover_path);

		void OnEnter() override;
		void OnPause() override;
		void OnResume() override;
		void OnExit() override;

		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		struct HitSpark {
			float cx = 0.f;
			float cy = 0.f;
			float age_ms = 0.f;
			std::uint32_t color = 0;
			int lane = 0;
		};
		std::vector<HitSpark> hit_sparks_;
		void SpawnHitFx(int lane, JudgeResult result);
		void UpdateHitFx(float delta_sec);
		void RenderHitFx();
		void ApplyPresentation(const JudgeCommand& cmd);

		uint8_t LaneCount() const { return GameConfig::kLaneCount; }
		void TryLoadCover();

		GameContext ctx_;
		std::string cover_path_;
		int cover_handle_ = -1;
		float retry_cooldown_ = 0.f;
		float song_time_ms_ = 0.f;
		GameLayout  layout_{};
		GameConfig::UiLayout ui_{};

		float progress_time_ms_ = 0.f;
		bool  is_in_outro_ = false;
		float audio_volume_ = 1.f;
		bool  result_pushed_ = false;

		JudgeResult last_judge_{};
		float judge_display_ms_ = 0.f;

		bool paused_ = false;
		bool is_in_lead_in_ = false;
		float lead_in_ms_ = 0.f;

		GameplaySession session_;
	};
}
