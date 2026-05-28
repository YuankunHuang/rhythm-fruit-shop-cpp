#include "ChartSelectScreen.h"
#include "LoadingScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include "../rhythm/SongDisplay.h"
#include "../platform/IRenderer.h"
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
		fallback_handle_ = ctx_.renderer.LoadTexture(GameConfig::kFallbackCoverPath);
	}

	void ChartSelectScreen::OnEnter() {
		StartPreviewForCurrentSong();
	}

	void ChartSelectScreen::OnResume() {
		// Only restart the preview state machine when bgm is not playing / back from Gameplay!
		if (ctx_.bgm.IsPlaying()) {
			return;
		}

		StartPreviewForCurrentSong();
	}

	void ChartSelectScreen::OnPause() {
		// Going into LoadingScreen — pause BGM so song audio can take over.
		//ctx_.bgm.Pause();
	}

	void ChartSelectScreen::Update(const FrameContext& ctx) {
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);

		// Adopt async cover once ready, trigger crossfade
		if (cover_pending_handle_ >= 0 && ctx_.renderer.IsTextureReady(cover_pending_handle_)) {
			float tw, th;
			bool valid = ctx_.renderer.GetTextureSize(cover_pending_handle_, tw, th) && tw > 0 && th > 0;
			int new_handle = valid ? cover_pending_handle_ : fallback_handle_;
			if (new_handle != cover_handle_) {
				old_cover_handle_ = cover_handle_;
				cover_handle_ = new_handle;
				crossfade_t_ = 0.f;
			}
			cover_pending_handle_ = -2;
		}

		crossfade_t_ = std::min(1.f, crossfade_t_ + ctx.delta_time * kCrossfadeSpeed);

		preview_timer_ms_ += ctx.delta_time * 1000.f;

		switch (preview_state_) {
		case PreviewState::Idle:
			preview_timer_ms_ = 0.f;
			preview_vol_ = 0.f;
			{
				const auto& song = catalog_.Songs()[selected_song_];
				ctx_.bgm.Stop();
				ctx_.bgm.SetLooping(false);
				ctx_.bgm.SetVolume(0.f);
				ctx_.bgm.LoadAsync(song.audio_path);  // non-blocking
			}
			preview_state_ = PreviewState::Loading;
			break;

		case PreviewState::Loading:
			if (ctx_.bgm.IsAudioReady()) {
				ctx_.bgm.SetVolume(0.f);
				ctx_.bgm.Seek(GameConfig::kPreviewStartMs);
				ctx_.bgm.Play();
				preview_vol_ = 0.f;
				preview_state_ = PreviewState::FadeIn;
				preview_timer_ms_ = 0.f;
			}
			break;

		case PreviewState::FadeIn:
			preview_vol_ = std::min(1.f, preview_timer_ms_ / GameConfig::kPreviewFadeInDuration);
			ctx_.bgm.SetVolume(preview_vol_);
			if (preview_timer_ms_ >= GameConfig::kPreviewFadeInDuration) {
				preview_state_ = PreviewState::Playing;
				preview_timer_ms_ = 0.f;
			}
			break;
		case PreviewState::FadeOut:
			preview_vol_ = std::max(0.f, 1.f - preview_timer_ms_ / GameConfig::kPreviewFadeOutDuration);
			ctx_.bgm.SetVolume(preview_vol_);
			if (preview_timer_ms_ >= GameConfig::kPreviewFadeOutDuration) {
				ctx_.bgm.Stop();
				preview_state_ = PreviewState::Idle;
			}
			break;
		case PreviewState::Playing:
			if (preview_timer_ms_ >= GameConfig::kPreviewDurationMs) {
				preview_state_ = PreviewState::FadeOut;
				preview_timer_ms_ = 0.f;
			}
			break;
		}
	}

	namespace {
		void DrawCoverFill(IRenderer& r, int handle, float win_w, float win_h, float alpha) {
			if (handle < 0) return;
			float tw = win_w, th = win_h;
			r.GetTextureSize(handle, tw, th);
			if (tw <= 0.f || th <= 0.f) return;
			float scale = std::max(win_w / tw, win_h / th);
			float dw = tw * scale, dh = th * scale;
			r.SubmitSprite((win_w - dw) * 0.5f, (win_h - dh) * 0.5f, dw, dh, handle, alpha);
		}
	}

	void ChartSelectScreen::Render() {
		const auto& ui = ui_;
		const float cx = ui.content_center_x;

		// Full-screen cover background with crossfade
		if (crossfade_t_ < 1.f) {
			if (old_cover_handle_ >= 0) {
				DrawCoverFill(ctx_.renderer, old_cover_handle_, ui.win_w, ui.win_h, 1.f - crossfade_t_);
			}
			DrawCoverFill(ctx_.renderer, cover_handle_, ui.win_w, ui.win_h, crossfade_t_);
			
		} else {
			DrawCoverFill(ctx_.renderer, cover_handle_, ui.win_w, ui.win_h, 1.f);
		}

		ctx_.renderer.SubmitQuad({ 0.f, 0.f, ui.win_w, ui.win_h, 0x00000080 });

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
					list_left - ui.font_body * 0.5f, y - row_h * 0.45f,
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
		const float diff_y = ui.content_bottom - ui.font_caption * 5.5f;
		const float tab_w = ui.font_body * 5.f;
		const float tabs_total = tab_w * diff_count;
		float tab_x = cx - tabs_total * 0.5f;

		for (int d = 0; d < diff_count; ++d) {
			bool is_active = (d == selected_diff_);
			std::string label(DiffDisplayName(song.difficulties[d]));
			uint32_t color = is_active ? GameColors::kJudgeLine : GameColors::kTextGray;
			if (is_active) {
				const float quad_w = tab_w - ui.font_body * 0.3f;
				ctx_.renderer.SubmitQuad({
					tab_x + (tab_w - quad_w) * 0.5f,
					diff_y - ui.font_body * 0.8f,
					quad_w,
					ui.font_body * 1.6f,
					GameColors::kPanelBg });
			}
			ctx_.renderer.SubmitText({
				tab_x + tab_w * 0.5f, diff_y,
				Anchor::Center, TextStyle::Body,
				label, color });
			tab_x += tab_w;
		}
	}

	// Speed: "Speed" label + 1/2/3/4 tabs grouped and centered together
	{
		static const char* kSpeedLabels[] = { "1", "2", "3", "4" };
		const float speed_y = ui.content_bottom - ui.font_caption * 2.5f;

		const int speed_count = std::size(GameConfig::kSpeedLevels);
		const float tab_w = ui.font_body * 2.f;
		const float label_w = ctx_.renderer.MeasureTextWidth("Speed ", TextStyle::Caption);
		const float group_total = label_w + tab_w * speed_count;
		float group_x = cx - group_total * 0.5f;

		ctx_.renderer.SubmitText({
			group_x, speed_y,
			Anchor::CenterLeft, TextStyle::Caption,
			"Speed", GameColors::kTextGray });

		float tab_x = group_x + label_w;
		for (int i = 0; i < speed_count; ++i) {
			bool is_active = (i == selected_speed_idx_);
			uint32_t color = is_active ? GameColors::kJudgeLine : GameColors::kTextGray;
			if (is_active) {
				const float quad_w = tab_w - ui.font_body * 0.3f;
				ctx_.renderer.SubmitQuad({
					tab_x + (tab_w - quad_w) * 0.5f,
					speed_y - ui.font_body * 0.8f,
					quad_w,
					ui.font_body * 1.6f,
					GameColors::kPanelBg });
			}
			ctx_.renderer.SubmitText({
				tab_x + tab_w * 0.5f, speed_y,
				Anchor::Center, TextStyle::Body,
				kSpeedLabels[i], color });
			tab_x += tab_w;
		}
	}

	// Hint bar
	ctx_.renderer.SubmitText({
		cx, ui.content_bottom,
		Anchor::BottomCenter, TextStyle::Caption,
		"ENTER Play    Up/Down Song    Left/Right Difficulty    ESC Back",
		GameColors::kTextHint, GameColors::kTextBlack });

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
			StartPreviewForCurrentSong();
			break;

		case InputAction::NavDown:
			if (song_count > 0) {
				selected_song_ = (selected_song_ + 1) % song_count;
				selected_diff_ = 0;
			}
			StartPreviewForCurrentSong();
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

		case InputAction::Enter:
			if (selected_song_ < song_count) {
				ConfirmSelection();
			}
			break;

		case InputAction::Escape:
			ctx_.ui.GoBack();
			break;

		case InputAction::Level1:
			selected_speed_idx_ = 0;
			break;

		case InputAction::Level2:
			selected_speed_idx_ = 1;
			break;

		case InputAction::Level3:
			selected_speed_idx_ = 2;
			break;

		case InputAction::Level4:
			selected_speed_idx_ = 3;
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
			song.audio_path,
			song.cover_path));
	}

	void ChartSelectScreen::StartPreviewForCurrentSong() {
		ctx_.bgm.Stop();
		preview_state_ = PreviewState::Idle;
		preview_timer_ms_ = 0.f;

		const auto& songs = catalog_.Songs();
		if (songs.empty()) return;
		const std::string& new_cover = songs[selected_song_].cover_path;
		if (new_cover == cover_path_loaded_) return;

		cover_path_loaded_ = new_cover;
		if (new_cover.empty()) {
			// No cover for this song — use fallback
			cover_pending_handle_ = -2;
			const int new_handle = fallback_handle_;
			if (new_handle != cover_pending_handle_) {
				old_cover_handle_ = cover_handle_;
				cover_handle_ = new_handle;
				crossfade_t_ = 0.f;
			}
		} else {
			// Start async load; keep showing current cover until ready
			cover_pending_handle_ = ctx_.renderer.LoadTextureAsync(new_cover);
			// If already cached, IsTextureReady returns true immediately;
			// Update() will adopt it on the very next frame.
		}
	}
}
