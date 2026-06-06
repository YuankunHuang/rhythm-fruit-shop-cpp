#pragma once

#include "../app/IScreen.h"
#include "GameContext.h"
#include "GameConfig.h"
#include "../rhythm/GameResult.h"
#include <string>

namespace rfs {
	class ResultScreen : public IScreen {
	public:
		ResultScreen(const GameContext& ctx, GameResult result, std::string cover_path);

		void OnEnter() override;
		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		void TryLoadCover();

		GameContext ctx_;
		GameResult  result_;
		std::string cover_path_;
		int cover_handle_ = -1;
		float retry_cooldown_ = 0.f;
		GameConfig::UiLayout ui_{};
	};
}
