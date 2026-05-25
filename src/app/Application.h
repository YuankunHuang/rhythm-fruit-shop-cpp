#pragma once

#include "../platform/IWindow.h"
#include "../platform/IRenderer.h"
#include "../platform/IInputSource.h"
#include "../rhythm/SmoothedSongClock.h"
#include "../platform/IAudioBackendClock.h"

namespace rfs {
	class Application {
	public:
		Application(IWindow& window, IInputSource& input, IRenderer& renderer, IAudioBackendClock& backend_clock, SmoothedSongClock& song_clock);
		bool Run();
	private:
		IWindow& window_;
		IInputSource& input_;
		IRenderer& renderer_;
		IAudioBackendClock& backend_clock_;
		SmoothedSongClock& song_clock_;
	};
}