#pragma once
#include <cstdint>
#include "GameConfig.h"

namespace rfs {

	// Screen-relative layout; recomputed each frame from window dimensions.
	// Pure data struct — no dependencies.
	struct GameLayout {
		float field_left = 0.f;
		float field_right = 0.f;
		float judge_y = 0.f;
		float spawn_y = 0.f;
		float lane_w = 0.f;
		float note_h = 0.f;

		float FieldHeight() const { return judge_y - spawn_y; }
		float LaneX(uint8_t lane) const { return field_left + lane * lane_w; }
		float LaneCenterX(uint8_t lane) const { return LaneX(lane) + lane_w * 0.5f; }

		// lane_count is read from the chart; layout adapts to any lane count.
		static GameLayout Compute(float win_w, float win_h, uint8_t lane_count) {
			if (lane_count < 1) lane_count = 1;
			float fw = win_w * GameConfig::kFieldWidthFrac;
			GameLayout L;
			L.field_left = (win_w - fw) * GameConfig::kFieldCenterFrac;
			L.field_right = L.field_left + fw;
			L.judge_y = win_h * GameConfig::kJudgeYFrac;
			L.spawn_y = win_h * GameConfig::kSpawnYFrac;
			L.lane_w = fw / static_cast<float>(lane_count);
			L.note_h = L.lane_w * GameConfig::kNoteHFrac;
			return L;
		}
	};

}
