#pragma once

#include "../app/IGameState.h"
#include "../platform/IRenderer.h"

namespace rfs {
	class MainMenuState : public IGameState {
	public:
		explicit MainMenuState(IRenderer& renderer);

		void Update(const FrameContext& ctx) override;
		void Render() override;
	private:
		IRenderer& renderer_;
	};
}