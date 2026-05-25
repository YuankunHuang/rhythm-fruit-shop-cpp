#include "SfmlRenderer.h"
#include <SFML/Graphics.hpp>
#include <string_view>
#include <string>

using namespace rfs;

struct SfmlRenderer::Impl {
	sf::RenderWindow* target;
	sf::Font font;
	sf::Text text;

	explicit Impl(sf::RenderWindow& window) : target(&window) {}
};

SfmlRenderer::SfmlRenderer(SfmlWindow& window) 
	: pimpl(std::make_unique<Impl>(window.RenderTarget()))
{
	if (!pimpl->font.loadFromFile("assets/fonts/Inter-Regular.TTF")) {
		throw std::runtime_error("Failed to load font");
	}
	pimpl->text.setFont(pimpl->font);
	pimpl->text.setCharacterSize(32);
}

SfmlRenderer::~SfmlRenderer() {
}

void SfmlRenderer::BeginFrame() {
}

void SfmlRenderer::Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
	pimpl->target->clear(sf::Color(r, g, b));
}

static sf::Color RgbaToSfmlColor(std::uint32_t rgba) {
	std::uint8_t r = (rgba >> 24) & 0xFF;
	std::uint8_t g = (rgba >> 16) & 0xFF;
	std::uint8_t b = (rgba >> 8) & 0xFF;
	std::uint8_t a = rgba & 0xFF;
	return sf::Color(r, g, b, a);
}

void SfmlRenderer::SubmitText(float x, float y, std::string_view text, std::uint32_t rgba) {
	pimpl->text.setString(std::string(text));
	pimpl->text.setPosition(x, y);
	pimpl->text.setFillColor(RgbaToSfmlColor(rgba));
	pimpl->target->draw(pimpl->text);
}

void SfmlRenderer::EndFrame() {
	pimpl->target->display();
}