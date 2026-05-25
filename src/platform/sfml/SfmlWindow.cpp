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
	return pimpl_->window;
}

SfmlWindow::SfmlWindow(unsigned w, unsigned h, const char* t) : pimpl_(std::make_unique<Impl>(w, h, t)) {}
bool SfmlWindow::IsOpen() const {
	return pimpl_->window.isOpen();
}
void SfmlWindow::Close() {
	pimpl_->window.close();
}
float SfmlWindow::Width() const {
	return static_cast<float>(pimpl_->window.getSize().x);
}
float SfmlWindow::Height() const {
	return static_cast<float>(pimpl_->window.getSize().y);
}