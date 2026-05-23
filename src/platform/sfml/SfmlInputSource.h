#pragma once

#include "../IInputSource.h"
#include <memory>
#include <span>

namespace rfs {
	class SfmlInputSource final : public IInputSource {
	public:
		SfmlInputSource();
		~SfmlInputSource();
		std::span<const InputEvent> Poll(HostNanos pollEnterHostNs) noexcept override;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl;
	};
}