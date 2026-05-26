#include "SfmlInputSource.h"
#include "SfmlWindow.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include "../InputEvent.h"
#include <array>
#include <optional>

using namespace rfs;

namespace {
	std::optional<InputAction> MapSfmlKeyToInputAction(sf::Keyboard::Key key) {
		switch (key) {
		case sf::Keyboard::D: return InputAction::Lane0;
		case sf::Keyboard::F: return InputAction::Lane1;
		case sf::Keyboard::J: return InputAction::Lane2;
		case sf::Keyboard::K: return InputAction::Lane3;
		case sf::Keyboard::Escape: return InputAction::Pause;
		case sf::Keyboard::Enter: return InputAction::Restart;
		case sf::Keyboard::F1: return InputAction::ToggleDebug;
		case sf::Keyboard::F2: return InputAction::CycleCalibration;
		case sf::Keyboard::Up: return InputAction::NavUp;
		case sf::Keyboard::Down: return InputAction::NavDown;
		case sf::Keyboard::Left: return InputAction::NavLeft;
		case sf::Keyboard::Right: return InputAction::NavRight;
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

std::span<const InputEvent> SfmlInputSource::Poll(HostNanos poll_enter_ns) noexcept {

	pimpl_->Clear(); // we don't want to keep old events around, clean start

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
				pimpl_->Push(action.value(), pressed, poll_enter_ns);
			}
		}
	}

	return { pimpl_->buffer.data(), pimpl_->count };
}
