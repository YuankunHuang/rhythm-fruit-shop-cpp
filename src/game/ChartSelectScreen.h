#pragma once

#include "../app/IScreen.h"
#include "../rhythm/ChartCatalog.h"
#include "GameContext.h"
#include "GameConfig.h"

namespace rfs {

	class ChartSelectScreen : public IScreen {
	public:
		explicit ChartSelectScreen(GameContext ctx);

		void OnEnter() override;
		void OnResume() override;
		void OnPause() override;

		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		void ConfirmSelection();

		GameContext ctx_;
		GameConfig::UiLayout ui_{};
		ChartCatalog catalog_{};
		bool catalog_ok_ = false;
		std::string catalog_error_;

		int selected_song_ = 0;
		int selected_diff_ = 0;

		// How many songs fit in the visible list area
		static constexpr int kVisibleRows = 7;
		int scroll_offset_ = 0;
	};

}
