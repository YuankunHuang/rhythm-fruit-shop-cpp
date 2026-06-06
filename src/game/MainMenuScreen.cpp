#include "MainMenuScreen.h"
#include "ChartSelectScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include "UiDraw.h"

namespace rfs {

	MainMenuScreen::MainMenuScreen(const GameContext& ctx) : ctx_(ctx) {}

	void MainMenuScreen::OnEnter() {
		StartBgm();
	}

	void MainMenuScreen::OnResume() {
		StartBgm();
	}

	void MainMenuScreen::StartBgm() {
		if (ctx_.bgm.Load(kBgmPath)) {
			ctx_.bgm.SetLooping(true);
			ctx_.bgm.Play();
		}
	}

	void MainMenuScreen::Update(const FrameContext& ctx) {
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);
	}

	void MainMenuScreen::Render() {
		const auto& ui = ui_;
		const float line_gap = ui.font_body * 1.6f;

		ctx_.renderer.SubmitText({
			ui.content_center_x, ui.content_center_y - line_gap,
			Anchor::Center, TextStyle::Title,
			"Rhythm Fruit Shop", GameColors::kTextWhite });

		// Key hints in a centered row: [ENTER] Select Song    [ESC] Quit
		const float cx = ui.content_center_x;
		const float hint_y = ui.content_center_y + line_gap;
		const float key_gap = ui.Px(8.f);
		const float hint_sep = ui.font_body * 1.4f;

		const float w_enter = UiDraw::MeasureKeyHint(ctx_.renderer, TextStyle::Caption, "ENTER", "Select Song", key_gap);
		const float w_esc = UiDraw::MeasureKeyHint(ctx_.renderer, TextStyle::Caption, "ESC", "Quit", key_gap);
		float x = cx - (w_enter + hint_sep + w_esc) * 0.5f;

		x += UiDraw::KeyHint(ctx_.renderer, x, hint_y, TextStyle::Caption,
			"ENTER", "Select Song", GameColors::kSelectAccent, GameColors::kTextGray,
			GameColors::kOutlineBlack, key_gap);
		x += hint_sep;
		UiDraw::KeyHint(ctx_.renderer, x, hint_y, TextStyle::Caption,
			"ESC", "Quit", GameColors::kSelectAccent, GameColors::kTextGray,
			GameColors::kOutlineBlack, key_gap);
	}

	void MainMenuScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;
		if (evt.action == InputAction::Enter) {
			ctx_.ui.NavigateTo(std::make_unique<ChartSelectScreen>(ctx_));
		}
		else if (evt.action == InputAction::Escape) {
			ctx_.window.Close();
		}
	}

}
