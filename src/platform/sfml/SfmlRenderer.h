#pragma once

#include "../IRenderer.h"
#include "SfmlWindow.h"
#include <memory>

namespace rfs {
	class SfmlRenderer final : public IRenderer {
	public:
		explicit SfmlRenderer(SfmlWindow& window);
		~SfmlRenderer();
		void BeginFrame() override;
		void Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) override;
		void SubmitText(float x, float y, std::string_view text, std::uint32_t rgba) override;
		void EndFrame() override;
	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl;
	};
}