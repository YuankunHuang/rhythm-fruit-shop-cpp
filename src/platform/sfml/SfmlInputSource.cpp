#include "SfmlInputSource.h"
#include "SfmlWindow.h"
#include <memory>
#include <SFML/Graphics.hpp>
#include "../InputEvent.h"
#include <array>
#include <optional>

using namespace rfs;

struct SfmlInputSource::Impl {
	SfmlWindow& window;
	std::array<InputEvent, kMaxEventsPerFrame> buffer{};
	std::size_t count = 0;

	explicit Impl(SfmlWindow& w) : window(w) {}

	void Clear() {
		count = 0; // no need to actually clear the buffer, just reset the count
	}

	void Push(InputAction action, bool pressed, HostNanos hostNs) {
		if (count >= buffer.size()) {
			return;
		}

		buffer[count++] = InputEvent{
			.action = action,
			.pressed = pressed,
			.eventHostNs = hostNs,
		};
	}
};

SfmlInputSource::SfmlInputSource(SfmlWindow& window) : pimpl(std::make_unique<Impl>(window)) {}

SfmlInputSource::~SfmlInputSource() = default;

static std::optional<InputAction> MapSfmlKeyToInputAction(sf::Keyboard::Key key) {
	switch (key) {
	case sf::Keyboard::D: return InputAction::Lane0;
	case sf::Keyboard::F: return InputAction::Lane1;
	case sf::Keyboard::J: return InputAction::Lane2;
	case sf::Keyboard::K: return InputAction::Lane3;
	case sf::Keyboard::Escape: return InputAction::Pause;
	case sf::Keyboard::Enter: return InputAction::Restart;
	case sf::Keyboard::F1: return InputAction::ToggleDebug;
	case sf::Keyboard::F2: return InputAction::CycleCalibration;
	default:
		return std::nullopt;
	}
}

std::span<const InputEvent> SfmlInputSource::Poll([[maybe_unused]] HostNanos ns) noexcept {

	pimpl->Clear(); // we don't want to keep old events around, clean start

	if (!pimpl->window.IsOpen()) {
		return {};
	}

	sf::Event evt{};
	while (pimpl->window.RenderTarget().pollEvent(evt)) {
		if (evt.type == sf::Event::Closed) {
			pimpl->window.Close();
			continue;
		}

		if (evt.type == sf::Event::KeyPressed || evt.type == sf::Event::KeyReleased) {
			const bool pressed = evt.type == sf::Event::KeyPressed;
			const auto action = MapSfmlKeyToInputAction(evt.key.code);
			if (action.has_value()) {
				pimpl->Push(action.value(), pressed, ns);
			}
		}
	}

	return { pimpl->buffer.data(), pimpl->count };
}
