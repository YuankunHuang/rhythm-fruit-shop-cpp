#include "SfmlInputSource.h"
#include "SfmlWindow.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include "../InputEvent.h"
#include <array>
#include <chrono>
#include <optional>

using namespace rfs;

namespace {
	std::optional<InputAction> MapSfmlKeyToInputAction(sf::Keyboard::Key key) {
		switch (key) {
		case sf::Keyboard::D: return InputAction::Lane0;
		case sf::Keyboard::F: return InputAction::Lane1;
		case sf::Keyboard::J: return InputAction::Lane2;
		case sf::Keyboard::K: return InputAction::Lane3;
		case sf::Keyboard::Escape: return InputAction::Escape;
		case sf::Keyboard::Enter: return InputAction::Enter;
		case sf::Keyboard::F1: return InputAction::ToggleDebug;
		case sf::Keyboard::Up: return InputAction::NavUp;
		case sf::Keyboard::Down: return InputAction::NavDown;
		case sf::Keyboard::Left: return InputAction::NavLeft;
		case sf::Keyboard::Right: return InputAction::NavRight;
		case sf::Keyboard::Num1: return InputAction::Level1;
		case sf::Keyboard::Num2: return InputAction::Level2;
		case sf::Keyboard::Num3: return InputAction::Level3;
		case sf::Keyboard::Num4: return InputAction::Level4;
		default:
			return std::nullopt;
		}
	}
}

struct SfmlInputSource::Impl {
	SfmlWindow& window;
	std::array<InputEvent, kMaxEventsPerFrame> buffer{};
	std::size_t count = 0;

	explicit Impl(SfmlWindow& w) : window(w) {}

	void Clear() {
		count = 0; // no need to actually clear the buffer, just reset the count
	}

	void Push(InputAction action, bool pressed, HostNanos host_ns) {
		if (count >= buffer.size()) {
			return;
		}

		buffer[count++] = InputEvent{
			.action = action,
			.pressed = pressed,
			.event_host_ns = host_ns,
		};
	}
};

SfmlInputSource::SfmlInputSource(SfmlWindow& window) : pimpl_(std::make_unique<Impl>(window)) {}

SfmlInputSource::~SfmlInputSource() = default;

std::span<InputEvent> SfmlInputSource::Poll([[maybe_unused]] HostNanos poll_enter_ns) noexcept {

	pimpl_->Clear();

	if (!pimpl_->window.IsOpen()) {
		return {};
	}

	sf::Event evt{};
	while (pimpl_->window.RenderTarget().pollEvent(evt)) {
		if (evt.type == sf::Event::Closed) {
			pimpl_->window.Close();
			continue;
		}

		if (evt.type == sf::Event::Resized) {
			pimpl_->window.OnResize(evt.size.width, evt.size.height);
			continue;
		}

		if (evt.type == sf::Event::KeyPressed || evt.type == sf::Event::KeyReleased) {
			const bool pressed = evt.type == sf::Event::KeyPressed;
			const auto action = MapSfmlKeyToInputAction(evt.key.code);
			if (action.has_value()) {
				const HostNanos event_ns = std::chrono::steady_clock::now().time_since_epoch().count();
				pimpl_->Push(action.value(), pressed, event_ns);
			}
		}
	}

	return { pimpl_->buffer.data(), pimpl_->count };
}
