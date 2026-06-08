#include "PauseScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include "UiDraw.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

namespace rfs {

	namespace {
		constexpr float kPanelHFactor = 2.8f;

		float PausePanelHeight(const GameConfig::UiLayout& ui) {
			return ui.font_body * kPanelHFactor;
		}

		HostNanos SteadyNowNs() {
			return std::chrono::steady_clock::now().time_since_epoch().count();
		}

		std::size_t FindCalibrationIndex(int32_t offset_ms) {
			const auto& steps = GameConfig::kCalibrationSteps;
			const std::size_t n = std::size(steps);
			for (std::size_t i = 0; i < n; ++i) {
				if (steps[i] == offset_ms) return i;
			}
			return 3;
		}

		std::string FormatOffsetMs(int32_t offset_ms) {
			if (offset_ms > 0) return "+" + std::to_string(offset_ms) + " ms";
			if (offset_ms < 0) return std::to_string(offset_ms) + " ms";
			return "0 ms";
		}

		void RenderTimingPanel(
			IRenderer& renderer,
			const GameConfig::UiLayout& ui,
			int32_t offset_ms,
			float cx,
			float top_y,
			float panel_h)
		{
			const std::size_t n = std::size(GameConfig::kCalibrationSteps);
			const std::size_t idx = FindCalibrationIndex(offset_ms);

			const float panel_w = std::clamp(ui.win_w * 0.42f, ui.Px(220.f), ui.Px(360.f));
			const float panel_x = cx - panel_w * 0.5f;

			// Opaque base so gameplay content never bleeds through the translucent panel.
			renderer.SubmitQuad({ panel_x, top_y, panel_w, panel_h, GameColors::kBgClear });
			renderer.SubmitQuad({ panel_x, top_y, panel_w, panel_h, GameColors::kPanelBg });
			UiDraw::RectOutline(renderer, panel_x, top_y, panel_w, panel_h, GameColors::kSelectAccent);

			// Row 1: "TIMING" label
			const float label_y = top_y + ui.font_body * 0.45f;
			renderer.SubmitText({
				cx, label_y, Anchor::TopCenter, TextStyle::Caption,
				"TIMING", GameColors::kSelectAccent, GameColors::kOutlineBlack });

			// Row 2: "< 0 ms >" stepper — arrows hide at range limits
			const float value_cy = top_y + ui.font_body * 1.85f;
			const uint32_t value_color = offset_ms == 0 ? GameColors::kTextWhite : GameColors::kSelectAccent;
			const std::string value_text = FormatOffsetMs(offset_ms);
			renderer.SubmitText({
				cx, value_cy, Anchor::Center, TextStyle::Body,
				value_text, value_color, GameColors::kOutlineBlack });

			const float value_half_w = renderer.MeasureTextWidth(value_text, TextStyle::Body) * 0.5f;
			const float arrow_gap = ui.Px(14.f);
			if (idx > 0) {
				renderer.SubmitText({
					cx - value_half_w - arrow_gap, value_cy, Anchor::CenterRight, TextStyle::Body,
					"<", GameColors::kSelectAccent, GameColors::kOutlineBlack });
			}
			if (idx < n - 1) {
				renderer.SubmitText({
					cx + value_half_w + arrow_gap, value_cy, Anchor::CenterLeft, TextStyle::Body,
					">", GameColors::kSelectAccent, GameColors::kOutlineBlack });
			}
		}

		void RenderSpeedPanel(
			IRenderer& renderer,
			const GameConfig::UiLayout& ui,
			int speed_idx,
			float cx,
			float top_y,
			float panel_h)
		{
			static const char* kSpeedLabels[] = { "1", "2", "3", "4" };
			const int speed_count = static_cast<int>(std::size(GameConfig::kSpeedLevels));

			const float panel_w = std::clamp(ui.win_w * 0.42f, ui.Px(220.f), ui.Px(360.f));
			const float panel_x = cx - panel_w * 0.5f;

			renderer.SubmitQuad({ panel_x, top_y, panel_w, panel_h, GameColors::kBgClear });
			renderer.SubmitQuad({ panel_x, top_y, panel_w, panel_h, GameColors::kPanelBg });
			UiDraw::RectOutline(renderer, panel_x, top_y, panel_w, panel_h, GameColors::kSelectAccent);

			const float label_y = top_y + ui.font_body * 0.45f;
			renderer.SubmitText({
				cx, label_y, Anchor::TopCenter, TextStyle::Caption,
				"SPEED", GameColors::kSelectAccent, GameColors::kOutlineBlack });

			const float pills_cy = top_y + ui.font_body * 1.85f;
			const float tab_w = ui.font_body * 2.f;
			const float pill_w = tab_w * 0.85f;
			const float pill_h = ui.font_body * 1.25f;
			const float group_w = tab_w * speed_count;
			float tab_x = cx - group_w * 0.5f;

			for (int i = 0; i < speed_count; ++i) {
				const bool is_active = (i == speed_idx);
				const float tab_cx = tab_x + tab_w * 0.5f;
				const float pill_x = tab_cx - pill_w * 0.5f;
				const float pill_y = pills_cy - pill_h * 0.5f;

				if (is_active) {
					renderer.SubmitQuad({
						pill_x, pill_y, pill_w, pill_h,
						GameColors::kSpeedPillFill });
					UiDraw::RectOutline(
						renderer,
						pill_x, pill_y, pill_w, pill_h,
						GameColors::kSpeedPillBorder);
				}

				const uint32_t color = is_active ? GameColors::kSelectAccent : GameColors::kTextGray;
				const uint32_t outline = is_active ? GameColors::kOutlineBlack : 0;

				renderer.SubmitText({
					tab_cx, pills_cy,
					Anchor::Center, TextStyle::Caption,
					kSpeedLabels[i], color, outline });

				tab_x += tab_w;
			}
		}
	}

	PauseScreen::PauseScreen(const GameContext& ctx) : ctx_(ctx) {}

	void PauseScreen::OnEnter() {
		const HostNanos host_now = SteadyNowNs();
		mode_ = PauseMode::Menu;
		countdown_ms_ = 0.f;
		was_playing_ = ctx_.audio.IsPlaying();
		ctx_.song_clock.SetFrozen(ctx_.song_clock.NowMs(host_now), host_now);
		if (was_playing_) ctx_.audio.Pause();
	}

	void PauseScreen::OnExit() {
		const HostNanos host_now = SteadyNowNs();
		if (!is_back_to_root_ && was_playing_) {
			ctx_.audio.Resume();
		}
		ctx_.song_clock.ClearFrozen(host_now);
	}

	void PauseScreen::Update(const FrameContext& ctx) {
		layout_ = GameLayout::Compute(ctx.win_w, ctx.win_h, GameConfig::kLaneCount);
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);

		if (mode_ == PauseMode::Countdown) {
			countdown_ms_ -= ctx.delta_time * 1000.f;
			if (countdown_ms_ <= 0.f) {
				is_back_to_root_ = false;
				ctx_.ui.GoBack();
			}
		}
	}

	void PauseScreen::Render() {
		const auto& L = layout_;
		const auto& ui = ui_;
		const float h = L.FieldHeight();
		const float cx = (L.field_left + L.field_right) * 0.5f;
		const float line_gap = ui.font_body * 1.5f;
		const float base_y = L.spawn_y + h * 0.38f;

		UiDraw::FullScreenDim(ctx_.renderer, ui.win_w, ui.win_h, GameColors::kOverlayMask);

		if (mode_ == PauseMode::Countdown) {
			UiDraw::FullScreenDim(ctx_.renderer, ui.win_w, ui.win_h, 0x00000044);

			const int seconds_left = static_cast<int>(std::ceil(countdown_ms_ / 1000.f));
			const std::string count_text = std::to_string(std::max(1, seconds_left));
			ctx_.renderer.SubmitText({
				cx, base_y, Anchor::Center, TextStyle::Title,
				count_text, GameColors::kSelectAccent, GameColors::kOutlineBlack });

			const float hint_y = base_y + ui.font_title * 1.4f;
			ctx_.renderer.SubmitText({
				cx, hint_y, Anchor::Center, TextStyle::Caption,
				"Press ESC to cancel", GameColors::kTextGray, GameColors::kOutlineBlack });
			return;
		}

		// Title in gold
		ctx_.renderer.SubmitText({ cx, base_y, Anchor::Center, TextStyle::Title,
			"PAUSED", GameColors::kSelectAccent, GameColors::kOutlineBlack });

		// Underline beneath title
		const float underline_y = base_y + ui.font_title * 0.62f;
		UiDraw::Underline(ctx_.renderer, cx, underline_y,
			ui.font_title * 2.2f, ui.Px(2.f), GameColors::kSelectAccent);

		const float panel_h = PausePanelHeight(ui);
		const float panel_gap = line_gap * 0.45f;
		const float hint_gap = line_gap * 0.95f;

		const float timing_top = underline_y + line_gap * 0.55f;
		const float speed_top = timing_top + panel_h + panel_gap;
		const float speed_bottom = speed_top + panel_h;
		const float resume_cy = speed_bottom + hint_gap;
		const float quit_cy = resume_cy + line_gap * 1.0f;

		float y_shift = 0.f;
		const float content_bottom = quit_cy + ui.font_caption * 0.5f;
		const float bottom_limit = ui.win_h - ui.Px(16.f);
		if (content_bottom > bottom_limit) {
			y_shift = content_bottom - bottom_limit;
		}

		RenderTimingPanel(ctx_.renderer, ui, ctx_.session.song_offset_ms, cx, timing_top - y_shift, panel_h);
		RenderSpeedPanel(ctx_.renderer, ui, ctx_.session.speed_idx, cx, speed_top - y_shift, panel_h);

		const float key_gap = ui.Px(8.f);

		// Resume hint (primary): white label
		const float resume_y = resume_cy - y_shift;
		const float resume_w = UiDraw::MeasureKeyHint(ctx_.renderer, TextStyle::Body, "ESC", "Resume", key_gap);
		UiDraw::KeyHint(ctx_.renderer, cx - resume_w * 0.5f, resume_y, TextStyle::Body,
			"ESC", "Resume", GameColors::kSelectAccent, GameColors::kTextWhite,
			GameColors::kOutlineBlack, key_gap);

		// Quit hint (destructive): red label
		const float quit_y = quit_cy - y_shift;
		const float quit_w = UiDraw::MeasureKeyHint(ctx_.renderer, TextStyle::Caption, "ENTER", "Quit to Menu", key_gap);
		UiDraw::KeyHint(ctx_.renderer, cx - quit_w * 0.5f, quit_y, TextStyle::Caption,
			"ENTER", "Quit to Menu", GameColors::kSelectAccent, GameColors::kMiss,
			GameColors::kOutlineBlack, key_gap);
	}

	void PauseScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;

		if (evt.action == InputAction::ToggleDebug && evt.pressed) {
			ctx_.session.show_debug_overlay = !ctx_.session.show_debug_overlay;
			return;
		}

		if (mode_ == PauseMode::Countdown) {
			if (evt.action == InputAction::Escape) {
				mode_ = PauseMode::Menu;
				countdown_ms_ = 0.f;
			}
			return;
		}

		switch (evt.action) {
		case InputAction::Escape:
			is_back_to_root_ = false;
			mode_ = PauseMode::Countdown;
			countdown_ms_ = GameConfig::kPauseResumeCountdownMs;
			break;
		case InputAction::Enter:
			is_back_to_root_ = true;
			ctx_.ui.GoBackToRoot();
			break;
		case InputAction::NavLeft:
		case InputAction::NavRight: {
			const auto& kSteps = GameConfig::kCalibrationSteps;
			const std::size_t n = std::size(kSteps);
			std::size_t i = FindCalibrationIndex(ctx_.session.song_offset_ms);
			i = std::min(i, n - 1);

			if (evt.action == InputAction::NavLeft) {
				if (i > 0) --i;
			}
			else {
				if (i + 1 < n) ++i;
			}
			ctx_.session.song_offset_ms = kSteps[i];
			break;
		}
		case InputAction::Level1:
			ctx_.session.speed_idx = 0;
			break;
		case InputAction::Level2:
			ctx_.session.speed_idx = 1;
			break;
		case InputAction::Level3:
			ctx_.session.speed_idx = 2;
			break;
		case InputAction::Level4:
			ctx_.session.speed_idx = 3;
			break;
		default:
			break;
		}
	}

}
