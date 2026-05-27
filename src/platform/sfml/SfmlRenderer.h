#pragma once

#include "../IRenderer.h"
#include "SfmlWindow.h"
#include <memory>
#include <string>

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
		void SubmitSprite(float x, float y, float w, float h, int texture_handle, float alpha = 1.f) override;
		float MeasureTextWidth(std::string_view text, TextStyle style) override;
		int LoadTexture(const std::string& path) override;
		int LoadTextureAsync(const std::string& path) override;
		void PollAsyncLoads() override;
		bool IsTextureReady(int handle) override;
		bool GetTextureSize(int handle, float& w, float& h) override;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl_;
	};
}