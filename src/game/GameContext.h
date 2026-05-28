#pragma once

#include "../platform/IWindow.h"
#include "../platform/IRenderer.h"
#include "../app/UIManager.h"
#include "../platform/IAudioPlayer.h"
#include "../rhythm/SmoothedSongClock.h"
#include "../game/PlaySessionConfig.h"

namespace rfs {
	struct GameContext {
		IWindow& window;
		IRenderer& renderer;
		IAudioPlayer& audio;
		IAudioPlayer& bgm;
		UIManager& ui;
		SmoothedSongClock& song_clock;
		PlaySessionConfig& session;
	};
}