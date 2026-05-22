#pragma once

#include "../app/IGameState.h"

namespace rfs {
	class MainMenuState : public IGameState {
	public:
		explicit MainMenuState(sf::RenderWindow& window);

		void Update(const FrameContext& ctx) override;
		void Render() override;

	private:

	};
}