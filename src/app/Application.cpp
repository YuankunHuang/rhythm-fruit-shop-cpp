#include "Application.h"
#include "StateStack.h"
#include "../game/MainMenuState.h"
#include <chrono>
#include <memory>

namespace rfs {
	Application::Application(IWindow& window, IInputSource& input, IRenderer& renderer) : 
		window_(window), input_(input), renderer_(renderer) { }

	bool Application::Run() {
		StateStack stack{};

		stack.Push(std::make_unique<MainMenuState>(renderer_, stack));

		while (window_.IsOpen()) {
			const auto poll_enter = std::chrono::steady_clock::now().time_since_epoch().count();
			auto evts = input_.Poll(poll_enter); // long long can be implicited converted to HostNanos, since they are both just aliases for std::int64_t
			for (const auto& evt : evts) {
				stack.Top().HandleInput(evt);
				if (!window_.IsOpen()) {
					break;
				}
			}

			FrameContext ctx{};
			auto& top_state = stack.Top();
			top_state.Update(ctx);

			renderer_.BeginFrame();
			renderer_.Clear(30, 30, 40);
			top_state.Render();
			renderer_.EndFrame();
		}
		return true;
	}
}