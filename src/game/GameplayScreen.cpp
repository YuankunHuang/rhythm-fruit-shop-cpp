#include "GameplayScreen.h"
#include "ResultScreen.h"
#include "PauseScreen.h"
#include "GameColors.h"
#include "GameRules.h"
#include "../platform/IRenderer.h"
#include <memory>
#include <algorithm>
#include <string>

namespace rfs {

	namespace {
		void DrawCoverFill(IRenderer& r, int handle, float win_w, float win_h, float alpha = 1.f) {
			if (handle < 0) return;
			float tw = win_w, th = win_h;
			r.GetTextureSize(handle, tw, th);
			if (tw <= 0.f || th <= 0.f) return;
			float scale = std::max(win_w / tw, win_h / th);
			float dw = tw * scale, dh = th * scale;
			r.SubmitSprite((win_w - dw) * 0.5f, (win_h - dh) * 0.5f, dw, dh, handle, alpha);
		}
	}

	GameplayScreen::GameplayScreen(GameContext ctx, FrozenChart chart, std::string cover_path)
		: ctx_(ctx), chart_(std::move(chart)), cover_path_(std::move(cover_path))
	{
		note_hit_.assign(chart_.Notes().size(), 0);
	}

	void GameplayScreen::Update(const FrameContext& ctx) {
		layout_ = GameLayout::Compute(ctx.win_w, ctx.win_h, LaneCount());
		ui_     = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);

		if (paused_) return;

		song_time_ms_ = ctx.song_time_ms;

		// Advance next_idx_: mark overdue notes as Miss
		const auto& notes = chart_.Notes();
		while (next_idx_ < static_cast<int>(notes.size()) &&
			song_time_ms_ - notes[next_idx_].time_ms > JudgeWindows::kGood) {
			if (!note_hit_[next_idx_]) {
				note_hit_[next_idx_] = 1;
				combo_ = 0;
				last_judge_ = JudgeResult::Miss;
				judge_display_ms_ = GameConfig::kJudgeDisplayMs;
				++cnt_miss_;
			}
			++next_idx_;
		}

		if (judge_display_ms_ > 0.f) {
			judge_display_ms_ -= ctx.delta_time * 1000.f;
		}

		// Song ending: wait for all notes to be processed, then delay before result
		if (!song_ending_ && next_idx_ >= static_cast<int>(notes.size())) {
			song_ending_ = true;
			end_timer_ms_ = GameConfig::kSongEndDelayMs;
		}
		if (song_ending_) {
			end_timer_ms_ -= ctx.delta_time * 1000.f;
			if (end_timer_ms_ <= 0.f && !result_pushed_) {
				result_pushed_ = true;
				ctx_.ui.ReplaceTop(std::make_unique<ResultScreen>(ctx_, BuildResult(), cover_path_));
			}
		}
	}

	void GameplayScreen::Render() {
		const auto& L = layout_;
		const uint8_t lane_count = LaneCount();

		// Cover background (lighter overlay so notes stay visible)
		int cover_handle = ctx_.renderer.LoadTexture(cover_path_);
		DrawCoverFill(ctx_.renderer, cover_handle, ui_.win_w, ui_.win_h);
		ctx_.renderer.SubmitQuad({ 0.f, 0.f, ui_.win_w, ui_.win_h, 0x00000088 });

		// Lane background strips
		for (uint8_t i = 0; i < lane_count; ++i) {
			ctx_.renderer.SubmitQuad({ L.LaneX(i), L.spawn_y, L.lane_w, L.FieldHeight(), GameColors::kLaneBg });
		}

		// Lane dividers + field borders
		for (uint8_t i = 1; i < lane_count; ++i) {
			float x = L.LaneX(i);
			ctx_.renderer.SubmitLine({ x, L.spawn_y, x, L.judge_y, GameColors::kLaneLine });
		}
		ctx_.renderer.SubmitLine({ L.field_left, L.spawn_y, L.field_left, L.judge_y, GameColors::kLaneLine });
		ctx_.renderer.SubmitLine({ L.field_right, L.spawn_y, L.field_right, L.judge_y, GameColors::kLaneLine });

		// Judge line glow + line
		ctx_.renderer.SubmitQuad({ L.field_left, L.judge_y - 6.f, L.field_right - L.field_left, 12.f, GameColors::kJudgeGlow });
		ctx_.renderer.SubmitLine({ L.field_left, L.judge_y, L.field_right, L.judge_y, GameColors::kJudgeLine });

		// Falling notes (start from next_idx_ — earlier notes are already resolved)
		float approach = static_cast<float>(chart_.ApproachTimeMs());
		const auto& notes = chart_.Notes();
		for (int i = next_idx_; i < static_cast<int>(notes.size()); ++i) {
			if (note_hit_[i]) continue;
			const auto& note = notes[i];
			float t = static_cast<float>(note.time_ms) - song_time_ms_;
			float norm = t / approach;
			if (norm < -0.15f || norm > 1.1f) continue;

			float ny = L.judge_y - norm * L.FieldHeight() - L.note_h;
			float nx = L.LaneX(note.lane);
			float pad = L.lane_w * 0.04f;
			uint32_t color = GameColors::kNoteColors[note.visual_id % GameColors::kNoteColorCount];
			ctx_.renderer.SubmitQuad({ nx + pad, ny, L.lane_w - pad * 2.f, L.note_h, color });
		}

		// Judgement text
		if (judge_display_ms_ > 0.f) {
			const char* text  = "";
			uint32_t    color = GameColors::kTextWhite;
			switch (last_judge_) {
			case JudgeResult::Perfect: text = "PERFECT"; color = GameColors::kPerfect; break;
			case JudgeResult::Great:   text = "GREAT";   color = GameColors::kGreat;   break;
			case JudgeResult::Good:    text = "GOOD";    color = GameColors::kGood;    break;
			case JudgeResult::Miss:    text = "MISS";    color = GameColors::kMiss;    break;
			}
			const float cx = (L.field_left + L.field_right) * 0.5f;
			ctx_.renderer.SubmitText({ cx, L.judge_y - ui_.font_hud * 2.f,
				Anchor::Center, TextStyle::Judge, text, color });
		}

		const float hud_x = ui_.content_right;
		const float hud_y_score = ui_.content_top;
		const float hud_y_combo = hud_y_score + ui_.font_hud * 1.5f;
		const std::string score_line = "SCORE " + std::to_string(score_);
		ctx_.renderer.SubmitText({ hud_x, hud_y_score, Anchor::TopRight, TextStyle::Hud, score_line, GameColors::kTextWhite });
		if (combo_ > 0) {
			const std::string combo_line = "COMBO " + std::to_string(combo_);
			ctx_.renderer.SubmitText({ hud_x, hud_y_combo, Anchor::TopRight, TextStyle::Hud, combo_line, GameColors::kTextGray });
		}
	}

	void GameplayScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;
		if (song_ending_) return;  // no input after song ends

		if (evt.action == InputAction::Escape) {
			ctx_.ui.NavigateTo(std::make_unique<PauseScreen>(ctx_));
			return;
		}

		int lane = -1;
		switch (evt.action) {
		case InputAction::Lane0: lane = 0; break;
		case InputAction::Lane1: lane = 1; break;
		case InputAction::Lane2: lane = 2; break;
		case InputAction::Lane3: lane = 3; break;
		default: return;
		}

		// Find nearest unhit note in this lane within Good window
		int best_idx = -1;
		int32_t best_dt = JudgeWindows::kGood + 1;
		const int32_t input_ms = evt.event_song_time_ms != 0
			? evt.event_song_time_ms
			: static_cast<int32_t>(song_time_ms_);
		const auto& notes = chart_.Notes();
		for (int i = next_idx_; i < static_cast<int>(notes.size()); ++i) {
			if (note_hit_[i]) continue;
			if (notes[i].lane != static_cast<uint8_t>(lane)) continue;
			int32_t dt = std::abs(notes[i].time_ms - input_ms);
			if (dt > JudgeWindows::kGood) {
				if (notes[i].time_ms - input_ms > JudgeWindows::kGood) break;
				continue;
			}
			if (dt < best_dt) { best_dt = dt; best_idx = i; }
		}

		if (best_idx < 0) return;

		note_hit_[best_idx] = 1;
		JudgeResult r =
			best_dt <= JudgeWindows::kPerfect ? JudgeResult::Perfect :
			best_dt <= JudgeWindows::kGreat ? JudgeResult::Great :
			JudgeResult::Good;

		++combo_;
		max_combo_ = std::max(max_combo_, combo_);
		score_ += Scoring::EarnScore(r, combo_);
		last_judge_ = r;
		judge_display_ms_ = GameConfig::kJudgeDisplayMs;

		switch (r) {
		case JudgeResult::Perfect: ++cnt_perfect_; break;
		case JudgeResult::Great: ++cnt_great_; break;
		case JudgeResult::Good: ++cnt_good_; break;
		default: break;
		}
	}

	GameResult GameplayScreen::BuildResult() const {
		return GameResult{
			.score = score_,
			.combo = max_combo_,
			.perfect = cnt_perfect_,
			.great = cnt_great_,
			.good = cnt_good_,
			.miss = cnt_miss_,
		};
	}

}
