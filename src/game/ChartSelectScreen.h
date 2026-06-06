#pragma once

#include "../app/IScreen.h"
#include "../rhythm/ChartCatalog.h"
#include "GameContext.h"
#include "GameConfig.h"

namespace rfs {

	class ChartSelectScreen : public IScreen {
	public:
		explicit ChartSelectScreen(const GameContext& ctx);

		void OnEnter() override;
		void OnResume() override;
		void OnPause() override;

		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;

	private:
		void ConfirmSelection();
		void StartPreviewForCurrentSong();

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

		// preview
		enum class PreviewState { Idle, Loading, FadeIn, Playing, FadeOut };
		PreviewState preview_state_ = PreviewState::Idle;
		float preview_timer_ms_ = 0.f;
		float preview_vol_ = 0.f;

		// cover
		int cover_handle_ = -1;         // currently displayed handle (may be fallback)
		int old_cover_handle_ = -1;     // previous handle during crossfade
		int fallback_handle_ = -1;      // cover-fallback.png, loaded once at startup
		int cover_pending_handle_ = -2; // -2 = no pending load
		float crossfade_t_ = 1.f;       // 0→1; 1 means crossfade complete
		std::string cover_path_loaded_;

		static constexpr float kCrossfadeSpeed = 1.f / 0.4f; // 0.4s
	};

}
