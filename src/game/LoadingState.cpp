#include "LoadingState.h"
#include "../platform/IRenderer.h"
#include "../rhythm/ChartLoader.h"
#include "GameplayState.h"

namespace rfs {
	LoadingState::LoadingState(IRenderer& renderer, StateStack& stack) 
		: renderer_(renderer), stack_(stack) {
		ChartLoader loader{};
		LoadError err{};
		auto chart = loader.Load("assets/charts/service/lemon-water-light.json", err);
		if (chart.has_value()) {
			load_ok_ = true;
			chart_ = std::move(chart);
			title_ = "Title: " + chart_->Title();
			detail_ = "Notes: " + std::to_string(chart_->Notes().size());
		} else {
			load_ok_ = false;
			title_ = "Failed to load chart";
			detail_ = "Error: " + err.code + " - " + err.message;
		}
	}

	void LoadingState::Update([[maybe_unused]] const FrameContext& ctx) {
	}

	void LoadingState::Render() {
		const std::uint32_t title_color = load_ok_ ? 0xFFFFFFFF : 0xFF6666FF;

		renderer_.SubmitText(220.f, 200.f, title_, title_color);
		renderer_.SubmitText(220.f, 260.f, detail_, 0xCCCCCCFF);

		if (load_ok_) {
			renderer_.SubmitText(220.f, 320.f, "Gameplay coming next :D", 0x888888FF);
		}
	}

	void LoadingState::HandleInput([[maybe_unused]] const InputEvent& evt) {
		if (!evt.pressed) {
			return;
		}

		if (evt.action == InputAction::Restart) {
			if (chart_.has_value()) {
				stack_.Push(std::make_unique<GameplayState>(renderer_, std::move(*chart_)));
			}
		}
	}
}