#include "DebugOverlay.h"
#include "GameColors.h"
#include "GameConfig.h"
#include <string>
#include <cmath>

namespace rfs {

	namespace {

		void DrawLine(
			IRenderer& r,
			float x, float y,
			Anchor anchor,
			TextStyle style,
			const std::string& text,
			std::uint32_t color)
		{
			r.SubmitText({ x, y, anchor, style, text, color });
		}

	}

	void RenderDebugOverlay(
		IRenderer& renderer,
		const GameConfig::UiLayout& ui,
		const PlaySessionConfig& session,
		std::size_t next_idx,
		float song_time_ms,
		int note_total)
	{
		const float x = ui.content_left;
		const float line_h = ui.font_caption * 1.4f;
		float y = ui.content_top;

		// translucent background
		const float panel_w = ui.win_w * 0.28f;
		const float panel_h = line_h * 6.2f;
		const float inset = ui.Px(8.f);
		renderer.SubmitQuad({ x - inset, y - inset, panel_w, panel_h, GameColors::kPanelBg });

		const auto draw = [&](const std::string& s) {
			DrawLine(renderer, x, y, Anchor::TopLeft, TextStyle::Caption, s, GameColors::kTextGray);
			y += line_h;
			};

		draw("DEBUG [F1]");

		// song time: keep 1 decimal point
		const int song_ms_i = static_cast<int>(std::lround(song_time_ms));
		draw("song: " + std::to_string(song_ms_i) + " ms");

		draw("offset: " + std::to_string(session.song_offset_ms) + " ms (pause: Left/Right)");

		const char* sign = session.last_judge_delta_ms >= 0 ? "+" : "";
		draw("last delta: " + std::string(sign) + std::to_string(session.last_judge_delta_ms) + " ms");

		draw("idx: " + std::to_string(next_idx) + " / " + std::to_string(note_total));

		if (session.last_frame_duration_ms > 0.f) {
			const int fps = static_cast<int>(std::lround(1000.f / session.last_frame_duration_ms));
			draw("frame: " + std::to_string(static_cast<int>(session.last_frame_duration_ms)) + " ms  (~" + std::to_string(fps) + " fps)");
		}
	}

}