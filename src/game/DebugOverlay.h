#pragma once

#include "../platform/IRenderer.h"
#include "PlaySessionConfig.h"
#include "GameConfig.h"

namespace rfs {
	void RenderDebugOverlay(IRenderer&, const GameConfig::UiLayout&, const PlaySessionConfig&, std::size_t next_idx, float song_time_ms, int note_total);
}