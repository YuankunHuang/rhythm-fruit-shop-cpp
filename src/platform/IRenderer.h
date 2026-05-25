#pragma once

#include <cstdint>
#include <string_view>

namespace rfs {
	class IRenderer {
	public:
		virtual ~IRenderer() = default;
		virtual void BeginFrame() = 0;
		virtual void Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) = 0;
		virtual void SubmitText(float x, float y, std::string_view text, std::uint32_t rgba) = 0;
		virtual void EndFrame() = 0;
	};
}