#include "GameplayScreen.h"
#include "ResultScreen.h"
#include "PauseScreen.h"
#include "GameColors.h"
#include "../platform/IRenderer.h"
#include "DebugOverlay.h"
#include <memory>
#include <algorithm>
#include <cmath>
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

	void GameplayScreen::SpawnHitFx(int lane, std::size_t note_index, JudgeResult result) {
		const auto& note = session_.Chart().Notes()[note_index];
		const uint32_t note_color =
			GameColors::kNoteColors[note.visual_id % GameColors::kNoteColorCount];
		const uint32_t judge_color = JudgeColor(result);

		auto h = hit_bursts_.Acquire();
		if (h.valid) {
			if (HitBurst* b = hit_bursts_.TryGet(h.index)) {
				b->lane = lane;
				b->cx = layout_.LaneCenterX(static_cast<uint8_t>(lane));
				b->cy = layout_.judge_y;
				b->age_ms = 0.f;
				b->judge_color = judge_color;
				b->note_color = note_color;
				b->result = result;
			}
		}

		if (lane >= 0 && lane < static_cast<int>(lane_pulses_.size())) {
			lane_pulses_[lane].age_ms = 0.f;
			lane_pulses_[lane].color = judge_color;
		}
	}

	void GameplayScreen::UpdateHitFx(float delta_sec) {
		const float dt_ms = delta_sec * 1000.f;

		hit_bursts_.ForEachActive([dt_ms](HitBurst& b) { b.age_ms += dt_ms; });
		hit_bursts_.ReleaseIf([](const HitBurst& b) {
			return b.age_ms >= GameConfig::kHitBurstLifetimeMs;
		});

		for (auto& p : lane_pulses_) {
			if (p.age_ms >= 0.f) {
				p.age_ms += dt_ms;
				if (p.age_ms >= GameConfig::kJudgePulseMs) {
					p.age_ms = -1.f;
				}
			}
		}
	}

	namespace {
		// Tier intensity for Perfect/Great/Good (Miss never spawns FX).
		float TierScale(JudgeResult r) {
			switch (r) {
			case JudgeResult::Perfect: return 1.0f;
			case JudgeResult::Great:   return 0.78f;
			case JudgeResult::Good:    return 0.55f;
			default:                   return 0.55f;
			}
		}

		// Eased 0->1 progress with clamp; returns 0 once past 1.
		float Phase(float age_ms, float dur_ms) {
			if (age_ms < 0.f || age_ms >= dur_ms) return -1.f;
			return age_ms / dur_ms;
		}
	}

	void GameplayScreen::RenderHitFx() {
		const float lane_w = layout_.lane_w;
		const float note_h = layout_.note_h;
		constexpr float kPi = 3.14159265f;

		hit_bursts_.ForEachActive([&](const HitBurst& b) {
			const float tier = TierScale(b.result);

			// 1. Lane vertical flash (0..kLaneFlashMs): whole lane column, low alpha.
			if (const float p = Phase(b.age_ms, GameConfig::kLaneFlashMs); p >= 0.f) {
				const float fade = 1.f - p;
				const float w = lane_w * 0.9f;
				ctx_.renderer.SubmitQuad({
					b.cx - w * 0.5f, layout_.spawn_y, w, layout_.FieldHeight(),
					GameColors::WithAlpha(b.judge_color, fade * 0.10f * tier) });
			}

			// 2. Note-colored pop (0..kNotePopMs): grows 1.0 -> 1.35, fades out.
			if (const float p = Phase(b.age_ms, GameConfig::kNotePopMs); p >= 0.f) {
				const float fade = 1.f - p;
				const float scale = 1.0f + p * 0.35f * tier;
				const float w = (lane_w * 0.9f) * scale;
				const float hgt = note_h * scale;
				ctx_.renderer.SubmitQuad({
					b.cx - w * 0.5f, b.cy - hgt * 0.5f, w, hgt,
					GameColors::WithAlpha(b.note_color, fade * 0.8f) });
			}

			// 3. Bright core (0..kHitCoreMs): small near-white square, high alpha.
			if (const float p = Phase(b.age_ms, GameConfig::kHitCoreMs); p >= 0.f) {
				const float fade = 1.f - p;
				const float s = lane_w * 0.22f * (0.6f + 0.4f * tier);
				ctx_.renderer.SubmitQuad({
					b.cx - s * 0.5f, b.cy - s * 0.5f, s, s,
					GameColors::WithAlpha(0xFFFFFF00u | 0xFFu, fade * 0.85f) });
			}

			// 4. Expanding square ring (0..kHitRingMs): 4 lines forming a growing box.
			auto draw_ring = [&](float p, float radius_scale, float alpha) {
				if (p < 0.f) return;
				const float fade = 1.f - p;
				const float r = lane_w * (0.18f + p * radius_scale * tier);
				const uint32_t c = GameColors::WithAlpha(b.judge_color, fade * alpha);
				const float l = b.cx - r, rt = b.cx + r, tp = b.cy - r, bt = b.cy + r;
				ctx_.renderer.SubmitLine({ l, tp, rt, tp, c });
				ctx_.renderer.SubmitLine({ rt, tp, rt, bt, c });
				ctx_.renderer.SubmitLine({ rt, bt, l, bt, c });
				ctx_.renderer.SubmitLine({ l, bt, l, tp, c });
			};
			draw_ring(Phase(b.age_ms, GameConfig::kHitRingMs), 0.85f, 0.8f);
			// Perfect-only outer ring, delayed ~40ms.
			if (b.result == JudgeResult::Perfect) {
				draw_ring(Phase(b.age_ms - 40.f, GameConfig::kHitRingMs), 1.15f, 0.5f);
			}

			// 5. Radial spokes (0..kHitSpokeMs): 6 lines from center outward.
			if (const float p = Phase(b.age_ms, GameConfig::kHitSpokeMs); p >= 0.f) {
				const float fade = 1.f - p;
				const float inner = lane_w * 0.10f;
				const float outer = lane_w * (0.25f + p * 0.6f * tier);
				const uint32_t c = GameColors::WithAlpha(b.judge_color, fade * 0.7f);
				constexpr int kSpokes = 6;
				for (int i = 0; i < kSpokes; ++i) {
					const float a = (static_cast<float>(i) / kSpokes) * 2.f * kPi;
					const float ca = std::cos(a), sa = std::sin(a);
					ctx_.renderer.SubmitLine({
						b.cx + ca * inner, b.cy + sa * inner,
						b.cx + ca * outer, b.cy + sa * outer, c });
				}
			}

			// 6. Judge-line horizontal sweep (0..kHitSweepMs): bright widening line.
			if (const float p = Phase(b.age_ms, GameConfig::kHitSweepMs); p >= 0.f) {
				const float fade = 1.f - p;
				const float half_w = lane_w * (0.35f + p * 0.45f) * (0.7f + 0.3f * tier);
				const uint32_t c = GameColors::WithAlpha(b.judge_color, fade * 0.9f);
				ctx_.renderer.SubmitLine({ b.cx - half_w, b.cy, b.cx + half_w, b.cy, c });
			}
		});
	}

	void GameplayScreen::RenderJudgeLine() {
		const auto& L = layout_;
		const float judge_glow_h = ui_.Px(12.f);
		const float judge_line_h = ui_.Px(6.f);

		// Per-lane base segments (same look as before, split per lane).
		for (uint8_t i = 0; i < LaneCount(); ++i) {
			const float x = L.LaneX(i);
			ctx_.renderer.SubmitQuad({ x, L.judge_y - judge_glow_h, L.lane_w, judge_glow_h * 2.f, GameColors::kJudgeGlow });
			ctx_.renderer.SubmitQuad({ x, L.judge_y - judge_line_h, L.lane_w, judge_line_h * 2.f, GameColors::kJudgeLine });
		}
	}

	void GameplayScreen::RenderJudgePulse() {
		const auto& L = layout_;
		const float judge_line_h = ui_.Px(6.f);

		for (uint8_t i = 0; i < LaneCount(); ++i) {
			const auto& p = lane_pulses_[i];
			if (p.age_ms < 0.f) continue;

			const float x = L.LaneX(i);
			// Front 80ms: bright flash; then decaying afterglow over kJudgePulseMs.
			float intensity;
			if (p.age_ms < 80.f) {
				intensity = 0.55f + 0.45f * (p.age_ms / 80.f);
			} else {
				const float t = (p.age_ms - 80.f) / (GameConfig::kJudgePulseMs - 80.f);
				intensity = std::clamp(1.f - t, 0.f, 1.f);
			}
			const float h = judge_line_h * (1.4f + 0.8f * intensity);
			ctx_.renderer.SubmitQuad({
				x, L.judge_y - h, L.lane_w, h * 2.f,
				GameColors::WithAlpha(p.color, intensity * 0.85f) });
		}
	}

	void GameplayScreen::ApplyPresentation(const JudgeCommand& cmd) {
		if (cmd.kind == JudgeCommand::Kind::TapHit) {
			const int lane = static_cast<int>(session_.Chart().Notes()[cmd.note_index].lane);
			SpawnHitFx(lane, cmd.note_index, cmd.result);
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

		RenderJudgeLine();

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
		RenderJudgePulse();

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

		if (auto taps = session_.HandleLaneTap(lane, input_ms)) {
			for (const auto& cmd : taps->Span()) {
				ctx_.session.last_judge_delta_ms = input_ms
					- (session_.Chart().Notes()[cmd.note_index].time_ms + session_.Config().song_offset_ms);
				ApplyPresentation(cmd);
			}
		}
	}
}
