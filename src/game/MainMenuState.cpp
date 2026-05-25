#include "MainMenuState.h"
#include "../app/IGameState.h"
#include "LoadingState.h"

namespace rfs {
	MainMenuState::MainMenuState(IRenderer& renderer, StateStack& stack) : renderer_(renderer), stack_(stack) {}

	void MainMenuState::Update([[maybe_unused]] const FrameContext& ctx) {
	}

	void MainMenuState::Render() {
		renderer_.SubmitText(220.f, 200.f, "Rhythm Fruit Shop", 0xFFFFFFFF);
		renderer_.SubmitText(260.f, 320.f, "Press Enter to Start", 0xCCCCCCFF);
	}

	void MainMenuState::HandleInput([[maybe_unused]] const InputEvent& evt) {
		if (!evt.pressed) {
			return;
		}

		if (evt.action == InputAction::Restart) {
			stack_.Push(std::make_unique<LoadingState>(renderer_));
		}
	}
}