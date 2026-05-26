#pragma once

#include "../app/IScreen.h"
#include "GameContext.h"
#include "GameConfig.h"
#include "GameResult.h"

namespace rfs {
	class ResultScreen : public IScreen {
	public:
		explicit ResultScreen(GameContext ctx, GameResult result);
		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;
	private:
		GameContext ctx_;
		GameResult  result_;
		GameConfig::UiLayout ui_{};
	};
}
