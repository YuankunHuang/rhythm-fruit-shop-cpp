#pragma once

#include "../app/IGameState.h"
#include "../platform/IRenderer.h"
#include <string>
#include <optional>
#include "../rhythm/FrozenChart.h"
#include "../app/StateStack.h"

namespace rfs {
	class LoadingState : public IGameState {
	public:
		explicit LoadingState(IRenderer& renderer, StateStack& stack);
		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;
	private:
		IRenderer& renderer_;
		StateStack& stack_;

		bool load_ok_ = false;
		std::string title_;
		std::string detail_;
		std::optional<FrozenChart> chart_;
	};
}