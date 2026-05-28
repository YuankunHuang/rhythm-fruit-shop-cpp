#include "LoadingScreen.h"
#include "GameplayScreen.h"
#include "GameColors.h"
#include "GameConfig.h"
#include "../rhythm/AudioPathResolver.h"
#include "../rhythm/SongDisplay.h"
#include "../platform/IRenderer.h"
#include <algorithm>
#include <chrono>
#include "UiDraw.h"

namespace rfs {

	LoadingScreen::LoadResult LoadingScreen::DoLoad(std::string path, std::string difficulty) {
		ChartLoader loader{};
		LoadError err{};
		auto chart = loader.Load(path, difficulty, err);

		LoadResult r{};
		if (chart.has_value()) {
			r.ok = true;
			r.chart = std::move(chart);
			const auto song_id = AudioPathResolver::SongIdFromChartPath(path);
			r.title = HumanizeSongId(song_id);
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
		std::string audio_path,
		std::string cover_path)
		: ctx_(ctx)
		, audio_path_(std::move(audio_path))
		, chart_path_(chart_path)
		, cover_path_(GameConfig::ResolveCoverPath(std::move(cover_path)))
	{
		future_ = std::async(std::launch::async, DoLoad, std::move(chart_path), std::move(difficulty));
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

				if (load_ok_) {
					const auto song_id = AudioPathResolver::SongIdFromChartPath(chart_path_);
					const auto resolved = AudioPathResolver::Resolve(song_id, audio_path_);
					if (resolved) {
						audio_path_ = resolved->string();
						detail_ += "   Audio: OK";
					}
					else {
						load_ok_ = false;
						title_ = "Audio not found";
						detail_ = "Could not resolve audio for: " + song_id;
					}
				}
			}
		}
	}

	void LoadingScreen::Render() {
		const auto& ui = ui_;
		const float cx = ui.content_center_x;
		const float cy = ui.content_center_y;
		const float line_gap = ui.font_body * 1.6f;

		// Cover background
		int cover_handle = ctx_.renderer.LoadTexture(cover_path_);
		UiDraw::CoverFill(ctx_.renderer, cover_handle, ui.win_w, ui.win_h);
		ctx_.renderer.SubmitQuad({ 0.f, 0.f, ui.win_w, ui.win_h, 0x000000B0 });

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
		ctx_.renderer.SubmitText({ cx, cy - line_gap * 1.2f, Anchor::Center, TextStyle::Title, title_, title_color });
		ctx_.renderer.SubmitText({ cx, cy, Anchor::Center, TextStyle::Body, detail_, GameColors::kTextGray });
		if (load_ok_) {
			ctx_.renderer.SubmitText({ cx, cy + line_gap * 1.2f, Anchor::Center, TextStyle::Caption,
				"Press Enter to play", GameColors::kTextHint });
			ctx_.renderer.SubmitText({ cx, cy + line_gap * 2, Anchor::Center, TextStyle::Caption,
				"Press Esc to go back", GameColors::kTextHint });
		}
		else {
			ctx_.renderer.SubmitText({ cx, cy + line_gap, Anchor::Center, TextStyle::Caption,
				"ESC  Back", GameColors::kTextHint });
		}
	}

	void LoadingScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;

		if (evt.action == InputAction::Escape) {
			ctx_.ui.GoBack();
			return;
		}

		if (!ready_ || !load_ok_) return;

		if (evt.action == InputAction::Enter && chart_.has_value()) {
			const auto song_id = AudioPathResolver::SongIdFromChartPath(chart_path_);
			const auto resolved = AudioPathResolver::Resolve(song_id, audio_path_);
			if (!resolved) {
				load_ok_ = false;
				title_ = "Audio not found";
				detail_ = "Could not resolve audio for: " + song_id;
				return;
			}

			ctx_.song_clock.Reset();
			ctx_.audio.Stop();
			if (!ctx_.audio.Load(*resolved)) {
				load_ok_ = false;
				title_ = "Audio load failed";
				detail_ = resolved->string();
				return;
			}

			ctx_.ui.ReplaceTop(std::make_unique<GameplayScreen>(ctx_, std::move(*chart_), cover_path_));
		}
	}

}
