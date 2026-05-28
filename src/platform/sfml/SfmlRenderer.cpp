#include "SfmlRenderer.h"
#include "../UiFontConfig.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <future>
#include <string>
#include <unordered_map>
#include <vector>

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
	std::unordered_map<std::string, int> texture_index_;
	std::vector<sf::Texture> textures_;
	std::vector<bool>        texture_ready_;

	struct PendingLoad {
		int handle;
		std::future<sf::Image> future;
	};
	std::vector<PendingLoad> pending_loads_;

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

	if (draw.outline_thickness > 0.f && draw.outline_rgba != 0) {
		pimpl_->text.setOutlineColor(RgbaToSfmlColor(draw.outline_rgba));
		pimpl_->text.setOutlineThickness(draw.outline_thickness * pimpl_->scale_);
	}
	else {
		pimpl_->text.setOutlineThickness(0.f);
	}

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

int SfmlRenderer::LoadTexture(const std::string& path) {
	if (path.empty())
		return -1;

	auto it = pimpl_->texture_index_.find(path);
	if (it != pimpl_->texture_index_.end()) {
		return it->second;
	}
	sf::Texture tex;
	if (!tex.loadFromFile(path)) {
		return -1;
	}
	int handle = static_cast<int>(pimpl_->textures_.size());
	pimpl_->textures_.push_back(std::move(tex));
	pimpl_->texture_ready_.push_back(true);
	pimpl_->texture_index_[path] = handle;
	return handle;
}

int SfmlRenderer::LoadTextureAsync(const std::string& path) {
	auto it = pimpl_->texture_index_.find(path);
	if (it != pimpl_->texture_index_.end()) {
		return it->second; // already cached — ready immediately
	}
	// Reserve a slot
	int handle = static_cast<int>(pimpl_->textures_.size());
	pimpl_->textures_.emplace_back();       // empty placeholder
	pimpl_->texture_ready_.push_back(false);
	pimpl_->texture_index_[path] = handle;

	// Load sf::Image on a background thread (no OpenGL context needed)
	auto future = std::async(std::launch::async, [path]() {
		sf::Image img;
		img.loadFromFile(path);
		return img;
	});
	pimpl_->pending_loads_.push_back({ handle, std::move(future) });
	return handle;
}

void SfmlRenderer::PollAsyncLoads() {
	auto& pending = pimpl_->pending_loads_;
	for (auto it = pending.begin(); it != pending.end(); ) {
		if (it->future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			sf::Image img = it->future.get();
			if (img.getSize().x > 0) {
				pimpl_->textures_[it->handle].loadFromImage(img);
			}
			pimpl_->texture_ready_[it->handle] = true;
			it = pending.erase(it);
		} else {
			++it;
		}
	}
}

bool SfmlRenderer::IsTextureReady(int handle) {
	if (handle < 0 || handle >= static_cast<int>(pimpl_->texture_ready_.size()))
		return false;
	return pimpl_->texture_ready_[handle];
}

bool SfmlRenderer::GetTextureSize(int handle, float& w, float& h) {
	if (handle < 0 || handle >= static_cast<int>(pimpl_->textures_.size()))
		return false;
	auto sz = pimpl_->textures_[handle].getSize();
	w = static_cast<float>(sz.x);
	h = static_cast<float>(sz.y);
	return true;
}

void SfmlRenderer::SubmitSprite(float x, float y, float w, float h, int texture_handle, float alpha) {
	if (texture_handle < 0 || texture_handle >= static_cast<int>(pimpl_->textures_.size()))
		return;
	const sf::Texture& tex = pimpl_->textures_[texture_handle];
	sf::Sprite sprite(tex);
	sprite.setPosition(x, y);
	sprite.setScale(sf::Vector2f(w / tex.getSize().x, h / tex.getSize().y));
	sprite.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha * 255)));
	pimpl_->target->draw(sprite);
}
