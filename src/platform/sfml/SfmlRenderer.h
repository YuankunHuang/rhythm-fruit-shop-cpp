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
		void EndFrame() override;
		void SubmitText(float x, float y, std::string_view text, std::uint32_t rgba) override;
		void SubmitLine(float x0, float y0, float x1, float y1, std::uint32_t rgba) override;
		void SubmitQuad(float x, float y, float w, float h, std::uint32_t rgba) override;
	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}