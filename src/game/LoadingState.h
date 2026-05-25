#pragma once

#include "../app/IGameState.h"
#include "../platform/IRenderer.h"
#include <string>
#include <optional>
#include "../rhythm/FrozenChart.h"

namespace rfs {
	class LoadingState : public IGameState {
	public:
		explicit LoadingState(IRenderer& renderer);
		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;
	private:
		IRenderer& renderer_;
		bool loadOk_ = false;
		std::string title_;
		std::string detail_;
		std::optional<FrozenChart> chart_;
	};
}