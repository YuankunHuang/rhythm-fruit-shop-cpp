#pragma once
#include "../app/IGameState.h"
#include "../app/FrameContext.h"
#include "../platform/IRenderer.h"
#include "../rhythm/FrozenChart.h"
#include "GameLayout.h"

namespace rfs {
	class GameplayState : public IGameState {
	public:
		explicit GameplayState(IRenderer& renderer, FrozenChart chart);
		void Update(const FrameContext& ctx) override;
		void Render() override;
		void HandleInput(const InputEvent& evt) override;
	
	private:
		IRenderer&  renderer_;
		FrozenChart chart_;
		float       song_time_ms_ = 0.f;
		GameLayout  layout_{};
	};
}