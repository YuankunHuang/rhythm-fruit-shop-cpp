#include "PauseScreen.h"
#include "GameColors.h"
#include "GameConfig.h"

namespace rfs {

	PauseScreen::PauseScreen(GameContext& ctx) : ctx_(ctx) {}

	void PauseScreen::OnEnter() {
		ctx_.audio.Pause();
	}

	void PauseScreen::OnExit() {
		ctx_.audio.Resume();
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
		ctx_.renderer.SubmitText({ cx, base_y + line_gap, Anchor::Center, TextStyle::Body,
			"Enter  Quit to Menu", GameColors::kTextGray });
		ctx_.renderer.SubmitText({ cx, base_y + line_gap * 2.f, Anchor::Center, TextStyle::Caption,
			"Esc    Resume", GameColors::kMiss });
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
