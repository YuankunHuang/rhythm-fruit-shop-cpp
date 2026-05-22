#include "Application.h"
#include "StateStack.h"
#include "../game/MainMenuState.h"
#include <memory>

namespace rfs {
	bool Application::Run() {
		StateStack stack{};


		sf::RenderWindow window(
			sf::VideoMode(800, 600),
			"Rhythm Fruit Shop"
		);

		while (window.isOpen()) {
			sf::Event evt{};
			while (window.pollEvent(evt)) {
				if (evt.type == sf::Event::Closed) {
					window.close();
				}
			}

			window.clear(sf::Color(30, 30, 40));
			window.display();
		}

		return true;
	}
}