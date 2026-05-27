#pragma once

#include "../app/IScreen.h"
#include "GameContext.h"
#include "GameConfig.h"
#include "GameResult.h"
#include <string>

namespace rfs {
	class ResultScreen : public IScreen {
	public:
		ResultScreen(GameContext ctx, GameResult result, std::string cover_path);
		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;
	private:
		GameContext ctx_;
		GameResult  result_;
		std::string cover_path_;
		GameConfig::UiLayout ui_{};
	};
}
