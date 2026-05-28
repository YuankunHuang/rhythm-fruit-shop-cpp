#include "SfmlWindow.h"
#include <SFML/Graphics.hpp>

using namespace rfs;

struct SfmlWindow::Impl {
	sf::RenderWindow window;
	Impl(unsigned w, unsigned h, const char* t)
		: window(sf::RenderWindow(sf::VideoMode(w, h), sf::String(t))) {
		window.setVerticalSyncEnabled(true);
	}
};

namespace {
	void SetPixelView(sf::RenderWindow& window, unsigned width, unsigned height) {
		sf::View view(sf::FloatRect(0.f, 0.f, static_cast<float>(width), static_cast<float>(height)));
		window.setView(view);
	}
}

SfmlWindow::~SfmlWindow() = default;

sf::RenderWindow& SfmlWindow::RenderTarget() {
	return pimpl_->window;
}

SfmlWindow::SfmlWindow(unsigned w, unsigned h, const char* t) : pimpl_(std::make_unique<Impl>(w, h, t)) {
	SetPixelView(pimpl_->window, w, h);
}
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
void SfmlWindow::OnResize(unsigned width, unsigned height) {
	SetPixelView(pimpl_->window, width, height);
}