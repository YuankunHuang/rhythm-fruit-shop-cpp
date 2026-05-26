#pragma once

#include "../app/IScreen.h"
#include "GameContext.h"
#include "GameLayout.h"
#include "GameConfig.h"

namespace rfs {
	class PauseScreen : public IScreen {
	public:
		explicit PauseScreen(GameContext& ctx);

		bool IsOverlay() const noexcept override { return true; }

		void OnEnter() override;
		void OnExit() override;

		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		GameContext ctx_;
		GameLayout  layout_{};
		GameConfig::UiLayout ui_{};
	};
}
