#include "GameplayScreen.h"
#include "ResultScreen.h"
#include "PauseScreen.h"
#include "GameColors.h"
#include "../platform/IRenderer.h"
#include "DebugOverlay.h"
#include <memory>
#include <algorithm>
#include <string>
#include "UiDraw.h"

namespace rfs {

	namespace {
		GameplaySessionConfig MakeSessionConfig(const PlaySessionConfig& session) {
			return GameplaySessionConfig{
				.song_offset_ms = session.song_offset_ms,
				.song_end_delay_ms = static_cast<std::int32_t>(GameConfig::kSongEndDelayMs),
			};
		}

		uint32_t JudgeColor(JudgeResult r) {
			switch (r) {
			case JudgeResult::Perfect: return GameColors::kPerfect;
			case JudgeResult::Great: return GameColors::kGreat;
			case JudgeResult::Good: return GameColors::kGood;
			default: return GameColors::kMiss;
			}
		}
	}

	GameplayScreen::GameplayScreen(const GameContext& ctx, FrozenChart chart, std::string cover_path)
		: ctx_(ctx)
		, cover_path_(std::move(cover_path))
		, session_(std::move(chart), MakeSessionConfig(ctx_.session))
	{
	}

	void GameplayScreen::OnEnter() {
		ctx_.bgm.Stop();
		is_in_lead_in_ = true;
		lead_in_ms_ = -GameConfig::kGameplayLeadInMs;
		TryLoadCover();
	}

	void GameplayScreen::TryLoadCover() {
		if (cover_handle_ >= 0) return;
		cover_handle_ = ctx_.renderer.LoadTexture(cover_path_);
		if (cover_handle_ < 0) {
			cover_handle_ = ctx_.renderer.LoadTexture(GameConfig::kFallbackCoverPath);
		}
	}

	void GameplayScreen::OnPause() {
		paused_ = true;
	}

	void GameplayScreen::OnResume() {
		paused_ = false;
	}

	void GameplayScreen::OnExit() {
		ctx_.audio.Stop();
		ctx_.song_clock.Reset();
	}

	void GameplayScreen::SpawnHitFx(int lane, JudgeResult result) {
		HitSpark s;
		s.lane = lane;
		s.cx = layout_.LaneCenterX(static_cast<uint8_t>(lane));
		s.cy = layout_.judge_y;
		s.age_ms = 0.f;
		s.color = JudgeColor(result);
		hit_sparks_.push_back(s);
	}

	void GameplayScreen::UpdateHitFx(float delta_sec) {
		const float dt_ms = delta_sec * 1000.f;
		for (auto& s : hit_sparks_) {
			s.age_ms += dt_ms;
		}
		std::erase_if(hit_sparks_, [](const HitSpark& s) {
			return s.age_ms >= GameConfig::kSparkLifetime;
		});
	}

	void GameplayScreen::RenderHitFx() {
		constexpr float kDurationMs = 280.f;

		for (const auto& s : hit_sparks_) {
			const float t = s.age_ms / kDurationMs;
			const float fade = 1.f - t;
			const float size = layout_.lane_w * (0.25f + t * 0.55f);

			ctx_.renderer.SubmitQuad({
				s.cx - size * 0.5f,
				s.cy - layout_.note_h * 0.5f,
				size,
				layout_.note_h * 0.9f,
				GameColors::WithAlpha(s.color, fade * 0.55f) });

			const float half_w = size * 0.7f;
			const uint32_t line_c = GameColors::WithAlpha(s.color, fade * 0.85f);
			ctx_.renderer.SubmitLine({ s.cx - half_w, s.cy, s.cx + half_w, s.cy, line_c });
		}
	}

	void GameplayScreen::ApplyPresentation(const JudgeCommand& cmd) {
		if (cmd.kind == JudgeCommand::Kind::TapHit) {
			const int lane = static_cast<int>(session_.Chart().Notes()[cmd.note_index].lane);
			SpawnHitFx(lane, cmd.result);
		}
		last_judge_ = cmd.result;
		judge_display_ms_ = GameConfig::kJudgeDisplayMs;
	}

	void GameplayScreen::Update(const FrameContext& ctx) {
		layout_ = GameLayout::Compute(ctx.win_w, ctx.win_h, LaneCount());
		ctx_.session.last_frame_duration_ms = ctx.delta_time * 1000.f;
		ui_ = GameConfig::UiLayout::Compute(ctx.win_w, ctx.win_h);

		if (retry_cooldown_ > 0.f) {
			retry_cooldown_ -= ctx.delta_time * 1000.f;
		}
		if (cover_handle_ < 0 && retry_cooldown_ <= 0.f) {
			TryLoadCover();
			retry_cooldown_ = 500.f;
		}

		if (paused_) return;

		const float chart_end_ms = static_cast<float>(session_.ChartEndMs());
		const float gameplay_end_ms = static_cast<float>(session_.GameplayEndMs());

		if (is_in_lead_in_) {
			lead_in_ms_ += ctx.delta_time * 1000.f;
			song_time_ms_ = lead_in_ms_;

			if (lead_in_ms_ >= 0.f) {
				is_in_lead_in_ = false;
				song_time_ms_ = 0.f;
				progress_time_ms_ = 0.f;
				ctx_.song_clock.Reset();
				audio_volume_ = 1.f;
				ctx_.audio.SetVolume(audio_volume_);
				ctx_.audio.Play();
			}
			return;
		}

		song_time_ms_ = ctx.song_time_ms;
		progress_time_ms_ += ctx.delta_time * 1000.f;
		progress_time_ms_ = std::max(progress_time_ms_, song_time_ms_);
		progress_time_ms_ = std::min(progress_time_ms_, chart_end_ms);

		UpdateHitFx(ctx.delta_time);

		if (!is_in_outro_ && progress_time_ms_ >= gameplay_end_ms) {
			is_in_outro_ = true;
		}

		if (is_in_outro_) {
			const float outro_t = std::clamp(
				(progress_time_ms_ - gameplay_end_ms) / GameConfig::kSongEndDelayMs,
				0.f, 1.f);
			audio_volume_ = 1.f - outro_t;
			ctx_.audio.SetVolume(audio_volume_);
		}

		const std::int32_t judge_time_ms = static_cast<std::int32_t>(std::lround(song_time_ms_));
		const auto misses = session_.Update(judge_time_ms);
		for (const auto& cmd : misses.Span()) {
			ApplyPresentation(cmd);
		}

		if (judge_display_ms_ > 0.f) {
			judge_display_ms_ -= ctx.delta_time * 1000.f;
		}

		if (!result_pushed_
			&& progress_time_ms_ >= chart_end_ms
			&& session_.Store().NextIdx() >= session_.Chart().Notes().size())
		{
			result_pushed_ = true;
			ctx_.ui.ReplaceTop(std::make_unique<ResultScreen>(ctx_, session_.Summary(), cover_path_));
		}
	}

	void GameplayScreen::Render() {
		const auto& L = layout_;
		const uint8_t lane_count = LaneCount();
		const float chart_end_ms = static_cast<float>(session_.ChartEndMs());

		UiDraw::CoverFill(ctx_.renderer, cover_handle_, ui_.win_w, ui_.win_h);
		ctx_.renderer.SubmitQuad({ 0.f, 0.f, ui_.win_w, ui_.win_h, 0x00000088 });

		for (uint8_t i = 0; i < lane_count; ++i) {
			ctx_.renderer.SubmitQuad({ L.LaneX(i), L.spawn_y, L.lane_w, L.FieldHeight(), GameColors::kLaneBg });
		}

		{
			const float field_w = L.field_right - L.field_left;
			const float band_h = std::max(ui_.Px(3.f), ui_.font_caption * 0.35f);
			const float band_y = L.spawn_y;
			const float progress = chart_end_ms > 0.f
				? std::clamp(progress_time_ms_ / chart_end_ms, 0.f, 1.f)
				: 0.f;

			ctx_.renderer.SubmitQuad({
				L.field_left, band_y, field_w, band_h,
				GameColors::kProgressTrack });

			if (progress > 0.f) {
				ctx_.renderer.SubmitQuad({
					L.field_left, band_y, field_w * progress, band_h,
					GameColors::kProgressFill });
			}
		}

		for (uint8_t i = 1; i < lane_count; ++i) {
			float x = L.LaneX(i);
			ctx_.renderer.SubmitLine({ x, L.spawn_y, x, L.judge_y, GameColors::kLaneLine });
		}
		ctx_.renderer.SubmitLine({ L.field_left, L.spawn_y, L.field_left, L.judge_y, GameColors::kLaneLine });
		ctx_.renderer.SubmitLine({ L.field_right, L.spawn_y, L.field_right, L.judge_y, GameColors::kLaneLine });

		const float judge_glow_h = ui_.Px(12.f);
		const float judge_line_h = ui_.Px(6.f);
		ctx_.renderer.SubmitQuad({ L.field_left, L.judge_y - judge_glow_h, L.field_right - L.field_left, judge_glow_h * 2.f, GameColors::kJudgeGlow });
		ctx_.renderer.SubmitQuad({ L.field_left, L.judge_y - judge_line_h, L.field_right - L.field_left, judge_line_h * 2.f, GameColors::kJudgeLine });

		float approach = static_cast<float>(GameConfig::kSpeedLevels[ctx_.session.speed_idx]);
		const auto& notes = session_.Chart().Notes();
		for (auto i = session_.Store().NextIdx(); i < notes.size(); ++i) {
			if (session_.Store().NoteResolved()[i]) continue;
			const auto& note = notes[i];
			float t = static_cast<float>(note.time_ms + ctx_.session.song_offset_ms) - song_time_ms_;
			float norm = t / approach;
			if (norm < -0.15f || norm > 1.1f) continue;

			float ny = L.judge_y - norm * L.FieldHeight() - L.note_h;
			float nx = L.LaneX(note.lane);
			float pad = L.lane_w * 0.04f;
			uint32_t color = GameColors::kNoteColors[note.visual_id % GameColors::kNoteColorCount];
			ctx_.renderer.SubmitQuad({ nx + pad, ny, L.lane_w - pad * 2.f, L.note_h, color });
		}

		RenderHitFx();

		const float cx = ui_.content_center_x;
		const float judge_anchor_y = ui_.content_top + ui_.win_h * 0.35f;

		if (!paused_ && judge_display_ms_ > 0.f && last_judge_ != JudgeResult::Miss) {
			const char* text = "";
			uint32_t color = GameColors::kTextWhite;
			switch (last_judge_) {
			case JudgeResult::Perfect:
				text = "PERFECT";
				color = GameColors::kPerfect;
				break;
			case JudgeResult::Great:
				text = "GREAT";
				color = GameColors::kGreat;
				break;
			case JudgeResult::Good:
				text = "GOOD";
				color = GameColors::kGood;
				break;
			default:
				break;
			}
			const float fade = std::clamp(judge_display_ms_ / GameConfig::kJudgeDisplayMs, 0.f, 1.f);
			uint32_t fill_c, outline_c;
			GameColors::TextColorsWithFade(color, GameColors::kOutlineBlack, fade, fill_c, outline_c);
			ctx_.renderer.SubmitText({
				cx, judge_anchor_y,
				Anchor::Center, TextStyle::Judge,
				text, fill_c, outline_c });
			if (session_.Score().Combo() > 1) {
				const std::string combo_line = std::to_string(session_.Score().Combo());
				GameColors::TextColorsWithFade(GameColors::kTextGold, GameColors::kOutlineBlack, fade, fill_c, outline_c);
				ctx_.renderer.SubmitText({
					cx, judge_anchor_y + ui_.font_hud * 1.4f,
					Anchor::Center, TextStyle::Hud,
					combo_line,
					fill_c,
					outline_c });
			}
		}

		if (!paused_ && judge_display_ms_ > 0.f && last_judge_ == JudgeResult::Miss) {
			const float fade = std::clamp(judge_display_ms_ / GameConfig::kJudgeDisplayMs, 0.f, 1.f);
			uint32_t fill_c, outline_c;
			GameColors::TextColorsWithFade(GameColors::kMiss, GameColors::kOutlineBlack, fade, fill_c, outline_c);
			ctx_.renderer.SubmitText({
				cx, judge_anchor_y + ui_.font_hud * 0.5f,
				Anchor::Center, TextStyle::Body,
				"MISS",
				fill_c,
				outline_c });
		}

		const float hud_x = ui_.content_right;
		ctx_.renderer.SubmitText({
			hud_x, ui_.content_top,
			Anchor::TopRight, TextStyle::Hud,
			"SCORE " + std::to_string(session_.Score().Score()),
			GameColors::kTextWhite, GameColors::kOutlineBlack });

		if (ctx_.session.show_debug_overlay) {
			RenderDebugOverlay(
				ctx_.renderer, ui_, ctx_.session,
				session_.Store().NextIdx(), song_time_ms_,
				static_cast<int>(session_.Chart().Notes().size()));
		}
	}

	void GameplayScreen::HandleInput(const InputEvent& evt) {
		if (!evt.pressed) return;

		if (evt.action == InputAction::ToggleDebug && evt.pressed) {
			ctx_.session.show_debug_overlay = !ctx_.session.show_debug_overlay;
			return;
		}

		if (evt.action == InputAction::Escape) {
			ctx_.ui.NavigateTo(std::make_unique<PauseScreen>(ctx_));
			return;
		}

		if (is_in_lead_in_ || is_in_outro_ || result_pushed_) return;

		int lane = -1;
		switch (evt.action) {
		case InputAction::Lane0: lane = 0; break;
		case InputAction::Lane1: lane = 1; break;
		case InputAction::Lane2: lane = 2; break;
		case InputAction::Lane3: lane = 3; break;
		default: return;
		}

		const std::int32_t input_ms = evt.event_song_time_ms != 0
			? evt.event_song_time_ms
			: static_cast<std::int32_t>(song_time_ms_);

		if (auto taps = session_.HandleLanePress(lane, input_ms)) {
			for (const auto& cmd : taps->Span()) {
				ctx_.session.last_judge_delta_ms = input_ms
					- (session_.Chart().Notes()[cmd.note_index].time_ms + session_.Config().song_offset_ms);
				ApplyPresentation(cmd);
			}
		}
	}
}
