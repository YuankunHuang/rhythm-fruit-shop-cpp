#include "LoadingState.h"
#include "../platform/IRenderer.h"
#include "../rhythm/ChartLoader.h"

namespace rfs {
	LoadingState::LoadingState(IRenderer& renderer) : renderer_(renderer) {
		ChartLoader loader{};
		LoadError err{};
		
		auto chart = loader.Load("assets/charts/demo_fruit_loop_01.json", err);
		if (chart.has_value()) {
			loadOk_ = true;
			chart_ = std::move(chart);
			title_ = "Title: " + chart_->Title();
			detail_ = "Notes: " + std::to_string(chart_->Notes().size());
		} else {
			loadOk_ = false;
			title_ = "Failed to load chart";
			detail_ = "Error: " + err.code + " - " + err.message;
		}
	}

	void LoadingState::Update([[maybe_unused]] const FrameContext& ctx) {
	}

	void LoadingState::Render() {
		const std::uint32_t titleColor = loadOk_ ? 0xFFFFFFFF : 0xFF6666FF; // ok: white, fail: light red

		renderer_.SubmitText(220.f, 200.f, title_, titleColor);
		renderer_.SubmitText(220.f, 260.f, detail_, 0xCCCCCCFF);

		if (loadOk_) {
			renderer_.SubmitText(220.f, 320.f, "Gameplay coming next :D", 0x888888FF);
		}
	}

	void LoadingState::HandleInput([[maybe_unused]] const InputEvent& evt) {}
}