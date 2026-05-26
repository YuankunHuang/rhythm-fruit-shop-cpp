#include "LoadingScreen.h"
#include "GameplayScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include <chrono>

namespace rfs {

	LoadingScreen::LoadResult LoadingScreen::DoLoad(std::string path, std::string difficulty) {
		ChartLoader loader{};
		LoadError err{};
		auto chart = loader.Load(path, difficulty, err);

		LoadResult r{};
		if (chart.has_value()) {
			r.ok = true;
			r.chart = std::move(chart);
			r.title = r.chart->Title();
			r.detail = "Notes: " + std::to_string(r.chart->Notes().size())
				+ "   Lanes: " + std::to_string(r.chart->LaneCount());
		}
		else {
			r.ok = false;
			r.title = "Failed to load chart";
			r.detail = err.code + ": " + err.message;
		}
		return r;
	}

	LoadingScreen::LoadingScreen(
		GameContext ctx,
		std::string chart_path,
		std::string difficulty,
		std::string audio_path)
		: ctx_(ctx)
		, audio_path_(std::move(audio_path))
	{
		future_ = std::async(std::launch::async, DoLoad,
			std::move(chart_path), std::move(difficulty));
	}

	void LoadingScreen::Update(const FrameContext& ctx) {
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);
		spin_ms_ += ctx.delta_time * 1000.f;

		if (!ready_ && future_.valid()) {
			if (future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
				auto result = future_.get();
				load_ok_ = result.ok;
				title_ = std::move(result.title);
				detail_ = std::move(result.detail);
				chart_ = std::move(result.chart);
				ready_ = true;
			}
		}
	}

	void LoadingScreen::Render() {
		const auto& ui = ui_;
		const float cx = ui.content_center_x;
		const float cy = ui.content_center_y;
		const float line_gap = ui.font_body * 1.6f;

		if (!ready_) {
			static const char* kFrames[] = { "/", "-", "\\", "|" };
			constexpr int kFrameCount = 4;
			const int frame = static_cast<int>(spin_ms_ / (GameConfig::kSpinnerPeriodMs / kFrameCount)) % kFrameCount;

			ctx_.renderer.SubmitText({
				cx, cy - line_gap,
				Anchor::Center, TextStyle::Body,
				"Loading", GameColors::kTextWhite });

			ctx_.renderer.SubmitText({
				cx, cy + line_gap,
				Anchor::Center, TextStyle::Hud,
				kFrames[frame], GameColors::kTextGray });
			return;
		}

		const uint32_t title_color = load_ok_ ? GameColors::kTextWhite : GameColors::kTextError;
		ctx_.renderer.SubmitText({ cx, cy - line_gap, Anchor::Center, TextStyle::Title, title_, title_color });
		ctx_.renderer.SubmitText({ cx, cy, Anchor::Center, TextStyle::Body, detail_, GameColors::kTextGray });
		if (load_ok_) {
			ctx_.renderer.SubmitText({ cx, cy + line_gap, Anchor::Center, TextStyle::Caption,
				"Press Enter to play", GameColors::kTextHint });
		}
		else {
			ctx_.renderer.SubmitText({ cx, cy + line_gap, Anchor::Center, TextStyle::Caption,
				"ESC  Back", GameColors::kTextHint });
		}
	}

	void LoadingScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;

		if (evt.action == InputAction::Pause) {
			ctx_.ui.GoBack();
			return;
		}

		if (!ready_) return;

		if (evt.action == InputAction::Restart && load_ok_ && chart_.has_value()) {
			ctx_.audio.Stop();
			ctx_.audio.Load(audio_path_);
			ctx_.song_clock.Reset();
			ctx_.audio.Play();
			ctx_.ui.ReplaceTop(std::make_unique<GameplayScreen>(ctx_, std::move(*chart_)));
		}
	}

}
