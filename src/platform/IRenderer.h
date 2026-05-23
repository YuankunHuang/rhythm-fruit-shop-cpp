#pragma once

#include <cstdint>

namespace rfs {
	class IRenderer {
	public:
		virtual ~IRenderer() = default;
		virtual void BeginFrame() = 0;
		virtual void Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) = 0;
		virtual void EndFrame() = 0;
	};
}