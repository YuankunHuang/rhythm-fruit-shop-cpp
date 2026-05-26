#include "SfmlRenderer.h"
#include "../UiFontConfig.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <string>

using namespace rfs;

namespace {

	sf::Color RgbaToSfmlColor(std::uint32_t rgba) {
		return sf::Color(
			static_cast<std::uint8_t>((rgba >> 24) & 0xFF),
			static_cast<std::uint8_t>((rgba >> 16) & 0xFF),
			static_cast<std::uint8_t>((rgba >> 8) & 0xFF),
			static_cast<std::uint8_t>(rgba & 0xFF));
	}

	unsigned CharSize(TextStyle style, float scale) {
		const float ref_h = UiFontConfig::kRefHeight;
		float frac = UiFontConfig::kBodyFrac;
		switch (style) {
		case TextStyle::Title:   frac = UiFontConfig::kTitleFrac;   break;
		case TextStyle::Body:    frac = UiFontConfig::kBodyFrac;    break;
		case TextStyle::Caption: frac = UiFontConfig::kCaptionFrac; break;
		case TextStyle::Hud:     frac = UiFontConfig::kHudFrac;     break;
		case TextStyle::Judge:   frac = UiFontConfig::kJudgeFrac;  break;
		}
		return static_cast<unsigned>(ref_h * scale * frac);
	}

	void ApplyAnchor(sf::Text& text, Anchor anchor, float ax, float ay) {
		const sf::FloatRect b = text.getLocalBounds();
		float ox = 0.f;
		float oy = 0.f;

		switch (anchor) {
		case Anchor::TopLeft:      break;
		case Anchor::TopCenter:    ox = b.left + b.width * 0.5f; break;
		case Anchor::TopRight:     ox = b.left + b.width; break;
		case Anchor::CenterLeft:   oy = b.top + b.height * 0.5f; break;
		case Anchor::Center:       ox = b.left + b.width * 0.5f; oy = b.top + b.height * 0.5f; break;
		case Anchor::CenterRight:  ox = b.left + b.width; oy = b.top + b.height * 0.5f; break;
		case Anchor::BottomLeft:   oy = b.top + b.height; break;
		case Anchor::BottomCenter: ox = b.left + b.width * 0.5f; oy = b.top + b.height; break;
		case Anchor::BottomRight:  ox = b.left + b.width; oy = b.top + b.height; break;
		}

		text.setOrigin(ox, oy);
		text.setPosition(ax, ay);
	}

} // namespace

struct SfmlRenderer::Impl {
	sf::RenderWindow* target;
	sf::Font          font;
	sf::Text          text;
	float             win_w   = 1280.f;
	float             win_h   = 720.f;
	float             scale_  = 1.f;

	explicit Impl(sf::RenderWindow& window) : target(&window) {}
};

SfmlRenderer::SfmlRenderer(SfmlWindow& window)
	: pimpl_(std::make_unique<Impl>(window.RenderTarget()))
{
	if (!pimpl_->font.loadFromFile("assets/fonts/Inter-Regular.ttf") &&
		!pimpl_->font.loadFromFile("assets/fonts/Inter-Regular.TTF")) {
		throw std::runtime_error("Failed to load font");
	}
	pimpl_->text.setFont(pimpl_->font);
}

SfmlRenderer::~SfmlRenderer() = default;

void SfmlRenderer::SetWindowSize(float win_w, float win_h) {
	pimpl_->win_w  = win_w;
	pimpl_->win_h  = win_h;
	const float sx = win_w / UiFontConfig::kRefWidth;
	const float sy = win_h / UiFontConfig::kRefHeight;
	pimpl_->scale_ = std::min(sx, sy);
}

void SfmlRenderer::BeginFrame() {}

void SfmlRenderer::Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
	pimpl_->target->clear(sf::Color(r, g, b));
}

void SfmlRenderer::EndFrame() {
	pimpl_->target->display();
}

void SfmlRenderer::SubmitText(const TextDraw& draw) {
	pimpl_->text.setCharacterSize(CharSize(draw.style, pimpl_->scale_));
	pimpl_->text.setString(std::string(draw.text));
	pimpl_->text.setFillColor(RgbaToSfmlColor(draw.rgba));
	ApplyAnchor(pimpl_->text, draw.anchor, draw.x, draw.y);
	pimpl_->target->draw(pimpl_->text);
}

float SfmlRenderer::MeasureTextWidth(std::string_view text, TextStyle style) {
	pimpl_->text.setCharacterSize(CharSize(style, pimpl_->scale_));
	pimpl_->text.setString(std::string(text));
	return pimpl_->text.getLocalBounds().width;
}

void SfmlRenderer::SubmitLine(const LineDraw& draw) {
	const auto color = RgbaToSfmlColor(draw.rgba);
	const sf::Vertex line[2] = {
		sf::Vertex(sf::Vector2f(draw.x0, draw.y0), color),
		sf::Vertex(sf::Vector2f(draw.x1, draw.y1), color),
	};
	pimpl_->target->draw(line, 2, sf::Lines);
}

void SfmlRenderer::SubmitQuad(const QuadDraw& draw) {
	const auto color = RgbaToSfmlColor(draw.rgba);
	const sf::Vertex quad[4] = {
		sf::Vertex(sf::Vector2f(draw.x, draw.y), color),
		sf::Vertex(sf::Vector2f(draw.x + draw.w, draw.y), color),
		sf::Vertex(sf::Vector2f(draw.x + draw.w, draw.y + draw.h), color),
		sf::Vertex(sf::Vector2f(draw.x, draw.y + draw.h), color),
	};
	pimpl_->target->draw(quad, 4, sf::Quads);
}
