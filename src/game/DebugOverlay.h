#pragma once

#include "../platform/IRenderer.h"
#include "PlaySessionConfig.h"
#include "../rhythm/JudgementSystem.h"
#include "GameConfig.h"

namespace rfs {
	void RenderDebugOverlay(IRenderer&, const GameConfig::UiLayout&, const PlaySessionConfig&, const GameplaySnapshot&, float song_time_ms, int note_total);
}