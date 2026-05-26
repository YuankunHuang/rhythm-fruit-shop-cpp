#pragma once

#include "../platform/IWindow.h"
#include "../platform/IRenderer.h"
#include "../app/UIManager.h"
#include "../platform/IAudioPlayer.h"
#include "../rhythm/SmoothedSongClock.h"

namespace rfs {
	struct GameContext {
		IWindow& window;
		IRenderer& renderer;
		IAudioPlayer& audio;
		IAudioPlayer& bgm;
		UIManager& ui;
		SmoothedSongClock& song_clock;
	};
}