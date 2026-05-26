#pragma once

#include "../IRenderer.h"
#include "SfmlWindow.h"
#include <memory>

namespace rfs {
	class SfmlRenderer final : public IRenderer {
	public:
		explicit SfmlRenderer(SfmlWindow& window);
		~SfmlRenderer();
		void SetWindowSize(float win_w, float win_h) override;
		void BeginFrame() override;
		void Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) override;
		void EndFrame() override;
		void SubmitText(const TextDraw& draw) override;
		void SubmitLine(const LineDraw& draw) override;
		void SubmitQuad(const QuadDraw& draw) override;
		float MeasureTextWidth(std::string_view text, TextStyle style) override;
	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}