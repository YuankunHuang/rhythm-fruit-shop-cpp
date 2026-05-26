#include "Application.h"
#include "UIManager.h"
#include "../game/MainMenuScreen.h"
#include "../game/GameContext.h"
#include "../game/GameColors.h"
#include <chrono>
#include <memory>

namespace rfs {

	Application::Application(
		IWindow& window, IInputSource& input, IRenderer& renderer,
		IAudioBackendClock& backend_clock, SmoothedSongClock& song_clock,
		IAudioPlayer& audio, IAudioPlayer& bgm) :
		window_(window), input_(input), renderer_(renderer),
		backend_clock_(backend_clock), song_clock_(song_clock), audio_(audio), bgm_(bgm) { }

	bool Application::Run() {
		UIManager ui{}; // ui has no interface

		GameContext ctx{ window_, renderer_, audio_, bgm_, ui, song_clock_ };
		ui.NavigateTo(std::make_unique<MainMenuScreen>(ctx));

		auto last_time = std::chrono::steady_clock::now();

		while (window_.IsOpen() && !ui.IsEmpty()) {
			auto now = std::chrono::steady_clock::now();
			float delta_sec = std::chrono::duration<float>(now - last_time).count();
			delta_sec = std::min(delta_sec, 0.1f);
			last_time = now;

			song_clock_.Tick(backend_clock_.Current());

			const auto poll_enter = std::chrono::steady_clock::now().time_since_epoch().count();
			auto evts = input_.Poll(poll_enter);
			for (const auto& evt : evts) {
				if (ui.IsEmpty()) break;
				ui.Top().HandleInput(evt);
				if (!window_.IsOpen()) break;
			}
			ui.FlushPending();  // execute deferred GoBack/GoBackToRoot after HandleInput

			if (ui.IsEmpty()) break;

			FrameContext frame{
				.delta_time   = delta_sec,
				.song_time_ms = song_clock_.NowMs(),
				.win_w = window_.Width(),
				.win_h = window_.Height(),
			};

			// Update only the top screen
			ui.Top().Update(frame);

			renderer_.SetWindowSize(frame.win_w, frame.win_h);
			renderer_.BeginFrame();
			renderer_.Clear(0x1E, 0x1E, 0x28);

			const auto& screens = ui.Screens();
			// Only overlay screens render through to the one below.
			int render_from = static_cast<int>(screens.size()) - 1;
			while (render_from > 0) {
				if (screens[render_from]->IsOverlay()) {
					--render_from;
				} else {
					break;
				}
			}
			for (int i = render_from; i < static_cast<int>(screens.size()); ++i) {
				screens[i]->Render();
			}

			renderer_.EndFrame();
		}
		return true;
	}

}
