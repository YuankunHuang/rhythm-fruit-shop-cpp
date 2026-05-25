#include "Application.h"
#include "StateStack.h"
#include "../game/MainMenuState.h"
#include <chrono>
#include <memory>

namespace rfs {
	Application::Application(
		IWindow& window,
		IInputSource& input,
		IRenderer& renderer,
		IAudioBackendClock& backend_clock,
		SmoothedSongClock& song_clock) :
		window_(window), input_(input), renderer_(renderer), backend_clock_(backend_clock), song_clock_(song_clock) { }

	bool Application::Run() {
		StateStack stack{};

		stack.Push(std::make_unique<MainMenuState>(renderer_, stack));

		while (window_.IsOpen()) {
			song_clock_.Tick(backend_clock_.Current());

			const auto poll_enter = std::chrono::steady_clock::now().time_since_epoch().count();
			auto evts = input_.Poll(poll_enter); // long long can be implicited converted to HostNanos, since they are both just aliases for std::int64_t
			for (const auto& evt : evts) {
				stack.Top().HandleInput(evt);
				if (!window_.IsOpen()) {
					break;
				}
			}

			FrameContext ctx{
				.song_time_ms = song_clock_.NowMs(),
				.win_w        = window_.Width(),
				.win_h        = window_.Height(),
			};
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