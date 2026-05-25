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
	: pimpl_(std::make_unique<Impl>(window.RenderTarget()))
{
	if (!pimpl_->font.loadFromFile("assets/fonts/Inter-Regular.TTF")) {
		throw std::runtime_error("Failed to load font");
	}
	pimpl_->text.setFont(pimpl_->font);
	pimpl_->text.setCharacterSize(32);
}

SfmlRenderer::~SfmlRenderer() {
}

void SfmlRenderer::BeginFrame() {
}

void SfmlRenderer::Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
	pimpl_->target->clear(sf::Color(r, g, b));
}

static sf::Color RgbaToSfmlColor(std::uint32_t rgba) {
	std::uint8_t r = (rgba >> 24) & 0xFF;
	std::uint8_t g = (rgba >> 16) & 0xFF;
	std::uint8_t b = (rgba >> 8) & 0xFF;
	std::uint8_t a = rgba & 0xFF;
	return sf::Color(r, g, b, a);
}

void SfmlRenderer::EndFrame() {
	pimpl_->target->display();
}

void SfmlRenderer::SubmitText(float x, float y, std::string_view text, std::uint32_t rgba) {
	pimpl_->text.setString(std::string(text));
	pimpl_->text.setPosition(x, y);
	pimpl_->text.setFillColor(RgbaToSfmlColor(rgba));
	pimpl_->target->draw(pimpl_->text);
}

void SfmlRenderer::SubmitLine(float x0, float y0, float x1, float y1, std::uint32_t rgba) {
	auto color = RgbaToSfmlColor(rgba);
	sf::Vertex line[2] = {
		sf::Vertex(sf::Vector2f(x0, y0), color),
		sf::Vertex(sf::Vector2f(x1, y1), color)
	};
	pimpl_->target->draw(line, 2, sf::Lines);
}

void SfmlRenderer::SubmitQuad(float x, float y, float w, float h, std::uint32_t rgba) {
	auto color = RgbaToSfmlColor(rgba);
	sf::Vertex quad[4] = {
		sf::Vertex(sf::Vector2f(x, y), color),
		sf::Vertex(sf::Vector2f(x + w, y), color),
		sf::Vertex(sf::Vector2f(x + w, y + h), color),
		sf::Vertex(sf::Vector2f(x, y + h), color),
	};
	pimpl_->target->draw(quad, 4, sf::Quads);
}
