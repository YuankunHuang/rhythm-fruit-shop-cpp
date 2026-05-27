#include "MainMenuScreen.h"
#include "ChartSelectScreen.h"
#include "GameColors.h"
#include "GameConfig.h"

namespace rfs {

	MainMenuScreen::MainMenuScreen(GameContext ctx) : ctx_(ctx) {}

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

		ctx_.renderer.SubmitText({
			ui.content_center_x, ui.content_center_y + line_gap,
			Anchor::Center, TextStyle::Caption,
			"ENTER: Select Song          ESC: Quit", GameColors::kTextGray });
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
