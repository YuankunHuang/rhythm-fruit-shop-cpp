#include "PauseScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include "UiDraw.h"
#include <algorithm>
#include <chrono>
#include <string>

namespace rfs {

	namespace {
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
			float top_y)
		{
			const std::size_t n = std::size(GameConfig::kCalibrationSteps);
			const std::size_t idx = FindCalibrationIndex(offset_ms);

			const float panel_w = std::clamp(ui.win_w * 0.42f, ui.Px(220.f), ui.Px(360.f));
			const float panel_h = ui.font_body * 2.8f;
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
	}

	PauseScreen::PauseScreen(const GameContext& ctx) : ctx_(ctx) {}

	void PauseScreen::OnEnter() {
		const HostNanos host_now = SteadyNowNs();
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
	}

	void PauseScreen::Render() {
		const auto& L = layout_;
		const auto& ui = ui_;
		const float h = L.FieldHeight();
		const float cx = (L.field_left + L.field_right) * 0.5f;
		const float line_gap = ui.font_body * 1.5f;
		const float base_y = L.spawn_y + h * 0.38f;

		// Full-screen overlay
		UiDraw::FullScreenDim(ctx_.renderer, ui.win_w, ui.win_h, GameColors::kOverlayMask);

		// Title in gold
		ctx_.renderer.SubmitText({ cx, base_y, Anchor::Center, TextStyle::Title,
			"PAUSED", GameColors::kSelectAccent, GameColors::kOutlineBlack });

		// Underline beneath title
		const float underline_y = base_y + ui.font_title * 0.62f;
		UiDraw::Underline(ctx_.renderer, cx, underline_y,
			ui.font_title * 2.2f, ui.Px(2.f), GameColors::kSelectAccent);

		// Timing panel — panel_h must match RenderTimingPanel's internal constant
		const float panel_h = ui.font_body * 2.8f;
		const float panel_top = underline_y + line_gap * 0.55f;
		RenderTimingPanel(ctx_.renderer, ui, ctx_.session.song_offset_ms, cx, panel_top);

		const float panel_bottom = panel_top + panel_h;
		const float key_gap = ui.Px(8.f);

		// Resume hint (primary): white label
		const float resume_cy = panel_bottom + line_gap * 0.95f;
		const float resume_w = UiDraw::MeasureKeyHint(ctx_.renderer, TextStyle::Body, "ESC", "Resume", key_gap);
		UiDraw::KeyHint(ctx_.renderer, cx - resume_w * 0.5f, resume_cy, TextStyle::Body,
			"ESC", "Resume", GameColors::kSelectAccent, GameColors::kTextWhite,
			GameColors::kOutlineBlack, key_gap);

		// Quit hint (destructive): red label
		const float quit_cy = resume_cy + line_gap * 1.0f;
		const float quit_w = UiDraw::MeasureKeyHint(ctx_.renderer, TextStyle::Caption, "ENTER", "Quit to Menu", key_gap);
		UiDraw::KeyHint(ctx_.renderer, cx - quit_w * 0.5f, quit_cy, TextStyle::Caption,
			"ENTER", "Quit to Menu", GameColors::kSelectAccent, GameColors::kMiss,
			GameColors::kOutlineBlack, key_gap);
	}

	void PauseScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;
		switch (evt.action) {
		case InputAction::Escape:
			is_back_to_root_ = false;
			ctx_.ui.GoBack();
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
		default:
			break;
		}
	}

}
