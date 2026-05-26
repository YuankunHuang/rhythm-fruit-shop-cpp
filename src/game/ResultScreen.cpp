#include "ResultScreen.h"
#include "../platform/IRenderer.h"
#include "GameColors.h"
#include "GameRules.h"
#include "GameConfig.h"
#include <algorithm>
#include <string>

namespace rfs {

	namespace {

		void DrawStatRow(rfs::IRenderer& renderer, float label_x, float value_x, float y,
			std::string_view label, std::string_view value, std::uint32_t value_color)
		{
			renderer.SubmitText({ label_x, y, Anchor::CenterLeft, TextStyle::Body, label, GameColors::kTextGray });
			renderer.SubmitText({ value_x, y, Anchor::CenterRight, TextStyle::Body, value, value_color });
		}

	}

	ResultScreen::ResultScreen(GameContext ctx, GameResult result)
		: ctx_(ctx), result_(result) {}

	void ResultScreen::Update(const FrameContext& ctx) {
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);
	}

	void ResultScreen::Render() {
		const auto& ui = ui_;

		const float panel_w = std::min(ui.win_w * 0.55f, 480.f);
		const float panel_h = ui.win_h * 0.72f;
		const float panel_x = (ui.win_w - panel_w) * 0.5f;
		const float panel_y = (ui.win_h - panel_h) * 0.5f;
		const float pad     = ui.win_w * 0.04f;
		const float label_x = panel_x + pad;
		const float value_x = panel_x + panel_w - pad;
		const float line_h  = ui.font_body * 1.9f;

		ctx_.renderer.SubmitQuad({ panel_x, panel_y, panel_w, panel_h, GameColors::kPanelBg });

		float y = panel_y + panel_h * 0.10f;
		ctx_.renderer.SubmitText({ ui.content_center_x, y, Anchor::TopCenter, TextStyle::Title,
			"RESULT", GameColors::kTextWhite });

		y += line_h * 1.6f;
		DrawStatRow(ctx_.renderer, label_x, value_x, y, "GRADE", std::string(Grading::CalcGrade(result_)), GameColors::kPerfect);
		y += line_h;
		DrawStatRow(ctx_.renderer, label_x, value_x, y, "SCORE", std::to_string(result_.score), GameColors::kTextWhite);
		y += line_h;
		DrawStatRow(ctx_.renderer, label_x, value_x, y, "MAX COMBO", std::to_string(result_.combo), GameColors::kTextGray);
		y += line_h;
		DrawStatRow(ctx_.renderer, label_x, value_x, y, "PERFECT", std::to_string(result_.perfect), GameColors::kPerfect);
		y += line_h;
		DrawStatRow(ctx_.renderer, label_x, value_x, y, "GREAT", std::to_string(result_.great), GameColors::kGreat);
		y += line_h;
		DrawStatRow(ctx_.renderer, label_x, value_x, y, "GOOD", std::to_string(result_.good), GameColors::kGood);
		y += line_h;
		DrawStatRow(ctx_.renderer, label_x, value_x, y, "MISS", std::to_string(result_.miss), GameColors::kMiss);

		ctx_.renderer.SubmitText({ ui.content_center_x, panel_y + panel_h - pad,
			Anchor::BottomCenter, TextStyle::Caption,
			"ENTER  Back to Song Select", GameColors::kTextHint });
	}

	void ResultScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;
		if (evt.action == InputAction::Restart) {
			ctx_.ui.GoBack();
		}
	}

}
