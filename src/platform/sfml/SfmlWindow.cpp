#include "SfmlWindow.h"
#include <SFML/Graphics.hpp>

using namespace rfs;

struct SfmlWindow::Impl {
	sf::RenderWindow window;
	Impl(unsigned w, unsigned h, const char* t) 
		: window(sf::RenderWindow(sf::VideoMode(w, h), sf::String(t))) {}
};

SfmlWindow::~SfmlWindow() = default;

sf::RenderWindow& SfmlWindow::RenderTarget() {
	return pimpl->window;
}

SfmlWindow::SfmlWindow(unsigned w, unsigned h, const char* t) : pimpl(std::make_unique<Impl>(w, h, t)) {}
bool SfmlWindow::IsOpen() const {
	return pimpl->window.isOpen();
}
void SfmlWindow::PollEvents() {
	sf::Event e;
	while (pimpl->window.pollEvent(e)) {
		if (e.type == sf::Event::Closed) {
			pimpl->window.close();
		}
	}
}
void SfmlWindow::Close() {
	pimpl->window.close();
}