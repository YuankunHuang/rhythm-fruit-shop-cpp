#include "GameplayState.h"

namespace rfs {

GameplayState::GameplayState(IRenderer& renderer, FrozenChart chart)
	: renderer_(renderer), chart_(std::move(chart)) { }

void GameplayState::Update(const FrameContext& ctx) {
	song_time_ms_ = ctx.song_time_ms;
	layout_ = GameLayout::Compute(ctx.win_w, ctx.win_h, chart_.LaneCount());
}

void GameplayState::Render() {
	const auto& L = layout_;
	const uint8_t lane_count = chart_.LaneCount();

	// Lane background strips
	for (uint8_t i = 0; i < lane_count; ++i) {
		renderer_.SubmitQuad(
			L.LaneX(i), L.spawn_y,
			L.lane_w, L.FieldHeight(),
			GameColors::kLaneBg);
	}

	// Lane dividers (vertical lines between lanes)
	for (uint8_t i = 1; i < lane_count; ++i) {
		float x = L.LaneX(i);
		renderer_.SubmitLine(x, L.spawn_y, x, L.judge_y, GameColors::kLaneLine);
	}

	// Field border lines (left / right edges)
	renderer_.SubmitLine(L.field_left,  L.spawn_y, L.field_left,  L.judge_y, GameColors::kLaneLine);
	renderer_.SubmitLine(L.field_right, L.spawn_y, L.field_right, L.judge_y, GameColors::kLaneLine);

	// Judge line glow (wide, low-alpha quad just above the line)
	renderer_.SubmitQuad(L.field_left, L.judge_y - 6.f,
		L.field_right - L.field_left, 12.f,
		GameColors::kJudgeGlow);

	// Judge line
	renderer_.SubmitLine(L.field_left, L.judge_y, L.field_right, L.judge_y, GameColors::kJudgeLine);

	// Falling notes
	float approach = static_cast<float>(chart_.ApproachTimeMs());
	for (const auto& note : chart_.Notes()) {
		float t    = static_cast<float>(note.time_ms) - song_time_ms_;
		float norm = t / approach;
		if (norm < -0.15f || norm > 1.1f) continue;

		float ny    = L.judge_y - norm * L.FieldHeight() - L.note_h;
		float nx    = L.LaneX(note.lane);
		float pad   = L.lane_w * 0.04f;
		uint32_t color = GameColors::kNoteColors[note.visual_id % GameColors::kNoteColorCount];

		renderer_.SubmitQuad(nx + pad, ny, L.lane_w - pad * 2.f, L.note_h, color);
	}
}

void GameplayState::HandleInput([[maybe_unused]] const InputEvent& evt) {
}

} // namespace rfs
