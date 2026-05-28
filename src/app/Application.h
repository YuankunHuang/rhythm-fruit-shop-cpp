#pragma once

#include "../platform/IWindow.h"
#include "../platform/IInputSource.h"
#include "../platform/IRenderer.h"
#include "../platform/IAudioBackendClock.h"
#include "../platform/IAudioPlayer.h"
#include "../rhythm/SmoothedSongClock.h"
#include "../game/PlaySessionConfig.h"

namespace rfs {
	class Application {
	public:
		Application(IWindow& window, IInputSource& input, IRenderer& renderer,
			IAudioBackendClock& backend_clock, SmoothedSongClock& song_clock,
			IAudioPlayer& audio, IAudioPlayer& bgm, PlaySessionConfig& session);
		bool Run();
	private:
		IWindow& window_;
		IInputSource& input_;
		IRenderer& renderer_;
		IAudioBackendClock& backend_clock_;
		SmoothedSongClock& song_clock_;
		IAudioPlayer& audio_;
		IAudioPlayer& bgm_;
		PlaySessionConfig& session_;
	};
}
