#include "SfmlRenderer.h"
#include <SFML/Graphics.hpp>

using namespace rfs;

struct SfmlRenderer::Impl {
	sf::RenderWindow* target;
};

SfmlRenderer::SfmlRenderer(SfmlWindow& window) 
	: pimpl(std::make_unique<Impl>())
{
	pimpl->target = &window.RenderTarget();
}

SfmlRenderer::~SfmlRenderer() {
}

void SfmlRenderer::BeginFrame() {
}

void SfmlRenderer::Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
	pimpl->target->clear(sf::Color(r, g, b));
}

void SfmlRenderer::EndFrame() {
	pimpl->target->display();
}