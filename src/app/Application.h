#pragma once

#include "../platform/IWindow.h"
#include "../platform/IRenderer.h"
#include "../platform/IInputSource.h"

namespace rfs {
	class Application {
	public:
		Application(IWindow& window, IInputSource& input, IRenderer& renderer);
		bool Run();
	private:
		IWindow& window_;
		IInputSource& input_;
		IRenderer& renderer_;
	};
}