#include "ResultScreen.h"
#include "../platform/IRenderer.h"
#include "GameColors.h"
#include "GameRules.h"
#include "GameConfig.h"
#include <algorithm>
#include <string>
#include "UiDraw.h"

namespace rfs {

	namespace {

		void DrawStatRow(rfs::IRenderer& renderer, float label_x, float value_x, float y,
			const std::string& label, const std::string& value, std::uint32_t value_color)
		{
			renderer.SubmitText({ label_x, y, Anchor::CenterLeft, TextStyle::Body, label, GameColors::kTextGray });
			renderer.SubmitText({ value_x, y, Anchor::CenterRight, TextStyle::Body, value, value_color });
		}

	}

	ResultScreen::ResultScreen(const GameContext& ctx, GameResult result, std::string cover_path)
		: ctx_(ctx), result_(result), cover_path_(std::move(cover_path)) {
	}

	void ResultScreen::OnEnter() {
		TryLoadCover();
	}

	void ResultScreen::TryLoadCover() {
		if (cover_handle_ >= 0) return;
		cover_handle_ = ctx_.renderer.LoadTexture(cover_path_);
		if (cover_handle_ < 0) {
			cover_handle_ = ctx_.renderer.LoadTexture(GameConfig::kFallbackCoverPath);
		}
	}

	void ResultScreen::Update(const FrameContext& ctx) {
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);

		if (retry_cooldown_ > 0.f) {
			retry_cooldown_ -= ctx.delta_time * 1000.f;
		}
		if (cover_handle_ < 0 && retry_cooldown_ <= 0.f) {
			TryLoadCover();
			retry_cooldown_ = 500.f;
		}
	}

	void ResultScreen::Render() {
		const auto& ui = ui_;

		UiDraw::CoverFill(ctx_.renderer, cover_handle_, ui.win_w, ui.win_h);
		ctx_.renderer.SubmitQuad({ 0.f, 0.f, ui.win_w, ui.win_h, 0x000000B0 });

		const float panel_w = ui.content_right - ui.content_left;
		const float panel_h = ui.content_bottom - ui.content_top;
		const float panel_x = ui.content_left;
		const float panel_y = ui.content_top;
		const float pad = ui.font_body * 0.8f;
		const float label_x = panel_x + pad;
		const float value_x = panel_x + panel_w - pad;
		const float line_h = ui.font_body * 1.9f;

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

		const float key_gap = ui.Px(8.f);
		const float hint_y = panel_y + panel_h - pad - ui.font_caption * 0.5f;
		const float w_back = UiDraw::MeasureKeyHint(ctx_.renderer, TextStyle::Caption, "ENTER", "Back to Song Select", key_gap);
		UiDraw::KeyHint(ctx_.renderer, ui.content_center_x - w_back * 0.5f, hint_y, TextStyle::Caption,
			"ENTER", "Back to Song Select", GameColors::kSelectAccent, GameColors::kTextGray,
			GameColors::kOutlineBlack, key_gap);
	}

	void ResultScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;
		if (evt.action == InputAction::Enter) {
			ctx_.ui.GoBack();
		}
	}

}
