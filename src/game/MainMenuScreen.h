#pragma once

#include "../app/IScreen.h"
#include "GameContext.h"
#include "GameLayout.h"
#include "GameConfig.h"

namespace rfs {
	class MainMenuScreen : public IScreen {
	public:
		explicit MainMenuScreen(const GameContext& ctx);

		void OnEnter() override;
		void OnResume() override;

		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		static constexpr const char* kBgmPath = "assets/audio/bgm/Open the Fruit Stand!_Loop.mp3";

		void StartBgm();

		GameContext ctx_;
		GameConfig::UiLayout ui_{};
	};
}
