#include "PauseScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include <chrono>

namespace rfs {

	namespace {
		HostNanos SteadyNowNs() {
			return std::chrono::steady_clock::now().time_since_epoch().count();
		}
	}

	PauseScreen::PauseScreen(GameContext& ctx) : ctx_(ctx) {}

	void PauseScreen::OnEnter() {
		const HostNanos host_now = SteadyNowNs();
		ctx_.song_clock.SetFrozen(ctx_.song_clock.NowMs(host_now), host_now);
		ctx_.audio.Pause();
	}

	void PauseScreen::OnExit() {
		const HostNanos host_now = SteadyNowNs();
		ctx_.audio.Resume();
		ctx_.song_clock.ClearFrozen(host_now);
	}

	void PauseScreen::Update(const FrameContext& ctx) {
		layout_ = GameLayout::Compute(ctx.win_w, ctx.win_h, GameConfig::kLaneCount);
		ui_     = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);
	}

	void PauseScreen::Render() {
		const auto& L  = layout_;
		const auto& ui = ui_;
		const float h  = L.FieldHeight();
		const float cx = (L.field_left + L.field_right) * 0.5f;
		const float line_gap = ui.font_body * 1.5f;
		const float base_y   = L.spawn_y + h * 0.38f;

		ctx_.renderer.SubmitQuad({ L.field_left, L.spawn_y,
			L.field_right - L.field_left, h, GameColors::kOverlayMask });

		ctx_.renderer.SubmitText({ cx, base_y, Anchor::Center, TextStyle::Title,
			"PAUSED", GameColors::kTextWhite });
		ctx_.renderer.SubmitText({ cx, base_y + line_gap * 2.f, Anchor::Center, TextStyle::Body,
			"Esc    Resume", GameColors::kTextGray });
		ctx_.renderer.SubmitText({ cx, base_y + line_gap * 3.f, Anchor::Center, TextStyle::Caption,
			"Enter  Quit to Menu", GameColors::kMiss });
	}

	void PauseScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;
		switch (evt.action) {
		case InputAction::Pause:
			ctx_.ui.GoBack();
			break;
		case InputAction::Restart:
			ctx_.ui.GoBackToRoot();
			break;
		default:
			break;
		}
	}

}
