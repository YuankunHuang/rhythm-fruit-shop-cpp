#pragma once

#include "../IInputSource.h"
#include "SfmlWindow.h"
#include <memory>
#include <span>

inline constexpr std::size_t kMaxEventsPerFrame = 64;

namespace rfs {
	class SfmlInputSource final : public IInputSource {
	public:
		SfmlInputSource(SfmlWindow& window);
		~SfmlInputSource();
		std::span<const InputEvent> Poll(HostNanos poll_enter_ns) noexcept override;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}