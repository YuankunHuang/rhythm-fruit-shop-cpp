#include "ChartSelectScreen.h"
#include "LoadingScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include "../rhythm/SongDisplay.h"
#include <string>
#include <algorithm>

namespace rfs {

	namespace {

		std::string_view DiffDisplayName(std::string_view key) {
			if (key == "easy")    return "Easy";
			if (key == "normal")  return "Normal";
			if (key == "hard")    return "Hard";
			if (key == "expert")  return "Expert";
			if (key == "service") return "Service";
			return key;
		}

	}

	ChartSelectScreen::ChartSelectScreen(GameContext ctx) : ctx_(ctx) {
		std::string err;
		catalog_ = ChartCatalog::Load("assets/charts/catalog.json", err);
		if (!catalog_.IsValid()) {
			catalog_error_ = err;
			catalog_ok_ = false;
		}
		else {
			catalog_ok_ = true;
		}
	}

	void ChartSelectScreen::OnEnter() {
		// BGM should already be playing from MainMenuScreen; resume if paused.
		if (!ctx_.bgm.IsPlaying()) {
			ctx_.bgm.Resume();
		}
	}

	void ChartSelectScreen::OnResume() {
		// Returning from a song — restart BGM playback.
		if (!ctx_.bgm.IsPlaying()) {
			ctx_.bgm.Resume();
		}
	}

	void ChartSelectScreen::OnPause() {
		// Going into LoadingScreen — pause BGM so song audio can take over.
		ctx_.bgm.Pause();
	}

	void ChartSelectScreen::Update(const FrameContext& ctx) {
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);
	}

	void ChartSelectScreen::Render() {
		const auto& ui = ui_;
		const float cx = ui.content_center_x;

		// Title bar
		ctx_.renderer.SubmitText({
			cx, ui.content_top,
			Anchor::TopCenter, TextStyle::Title,
			"Select Song", GameColors::kTextWhite });

		if (!catalog_ok_) {
			ctx_.renderer.SubmitText({
				cx, ui.content_center_y,
				Anchor::Center, TextStyle::Body,
				catalog_error_, GameColors::kTextError });
			return;
		}

		const auto& songs = catalog_.Songs();

		if (songs.empty()) {
			ctx_.renderer.SubmitText({
				cx, ui.content_center_y,
				Anchor::Center, TextStyle::Body,
				"No songs found. Run 03_import_for_cpp.bat to import songs.",
				GameColors::kTextGray });
			ctx_.renderer.SubmitText({
				cx, ui.content_bottom,
				Anchor::BottomCenter, TextStyle::Caption,
				"ESC  Main Menu", GameColors::kTextHint });
			return;
		}

		// Song list
		const float list_top = ui.content_top + ui.font_title * 2.f;
		const float row_h = ui.font_body * 2.2f;
		const float list_left = ui.content_left;
		const float list_right = cx - ui.font_body;

		const int song_count = static_cast<int>(songs.size());
		// Keep scroll so selected is visible
		scroll_offset_ = std::clamp(scroll_offset_,
			selected_song_ - kVisibleRows + 1,
			selected_song_);
		scroll_offset_ = std::max(0, scroll_offset_);

		for (int i = 0; i < kVisibleRows; ++i) {
			int song_idx = scroll_offset_ + i;
			if (song_idx >= song_count) break;

			float y = list_top + i * row_h;
			bool is_selected = (song_idx == selected_song_);

			if (is_selected) {
				// Highlight box
				ctx_.renderer.SubmitQuad({
					list_left - ui.font_body * 0.5f, y - row_h * 0.35f,
					list_right - list_left + ui.font_body,
					row_h * 0.9f,
					GameColors::kPanelBg });
			}

			uint32_t color = is_selected ? GameColors::kTextWhite : GameColors::kTextGray;
			TextStyle style = is_selected ? TextStyle::Body : TextStyle::Caption;
			ctx_.renderer.SubmitText({
				list_left, y,
				Anchor::CenterLeft, style,
				DisplaySongTitle(songs[song_idx]), color });
		}

		// Difficulty tabs for selected song
		if (selected_song_ < song_count) {
			const auto& song = songs[selected_song_];
			const int diff_count = static_cast<int>(song.difficulties.size());
			const float diff_y = ui.content_bottom - ui.font_body * 3.5f;
			const float tab_w = ui.font_body * 5.f;
			const float tabs_total = tab_w * diff_count;
			float tab_x = cx - tabs_total * 0.5f;

			for (int d = 0; d < diff_count; ++d) {
				bool is_active = (d == selected_diff_);
				std::string label(DiffDisplayName(song.difficulties[d]));
				uint32_t color = is_active ? GameColors::kJudgeLine : GameColors::kTextGray;
				if (is_active) {
					ctx_.renderer.SubmitQuad({
						tab_x, diff_y - ui.font_body * 0.6f,
						tab_w - ui.font_body * 0.3f,
						ui.font_body * 1.4f,
						GameColors::kPanelBg });
				}
				ctx_.renderer.SubmitText({
					tab_x + tab_w * 0.5f, diff_y,
					Anchor::Center, TextStyle::Body,
					label, color });
				tab_x += tab_w;
			}
		}

		// Hint bar
		ctx_.renderer.SubmitText({
			cx, ui.content_bottom,
			Anchor::BottomCenter, TextStyle::Caption,
			"ENTER Play    Up/Down Song    Left/Right Difficulty    ESC Back",
			GameColors::kTextHint });
	}

	void ChartSelectScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;

		const auto& songs = catalog_.Songs();
		const int song_count = static_cast<int>(songs.size());

		switch (evt.action) {
		case InputAction::NavUp:
			if (song_count > 0) {
				selected_song_ = (selected_song_ - 1 + song_count) % song_count;
				selected_diff_ = 0;
			}
			break;

		case InputAction::NavDown:
			if (song_count > 0) {
				selected_song_ = (selected_song_ + 1) % song_count;
				selected_diff_ = 0;
			}
			break;

		case InputAction::NavLeft:
			if (selected_song_ < song_count) {
				const int diff_count = static_cast<int>(songs[selected_song_].difficulties.size());
				if (diff_count > 0) {
					selected_diff_ = (selected_diff_ - 1 + diff_count) % diff_count;
				}
			}
			break;

		case InputAction::NavRight:
			if (selected_song_ < song_count) {
				const int diff_count = static_cast<int>(songs[selected_song_].difficulties.size());
				if (diff_count > 0) {
					selected_diff_ = (selected_diff_ + 1) % diff_count;
				}
			}
			break;

		case InputAction::Restart:
			if (selected_song_ < song_count) {
				ConfirmSelection();
			}
			break;

		case InputAction::Pause:
			ctx_.ui.GoBack();
			break;

		default:
			break;
		}
	}

	void ChartSelectScreen::ConfirmSelection() {
		const auto& songs = catalog_.Songs();
		if (songs.empty()) return;
		const auto& song = songs[selected_song_];
		if (song.difficulties.empty()) return;

		const int diff_idx = std::clamp(selected_diff_, 0, static_cast<int>(song.difficulties.size()) - 1);
		const std::string& difficulty = song.difficulties[diff_idx];

		ctx_.ui.NavigateTo(std::make_unique<LoadingScreen>(
			ctx_,
			song.chart_path,
			difficulty,
			song.audio_path));
	}

}
