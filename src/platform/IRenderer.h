#pragma once

#include <cstdint>
#include <string_view>
#include <string>

namespace rfs {

	enum class Anchor {
		TopLeft, TopCenter, TopRight,
		CenterLeft, Center, CenterRight,
		BottomLeft, BottomCenter, BottomRight
	};

	enum class TextStyle {
		Title, Body, Caption, Hud, Judge
	};

	struct TextDraw {
		float x, y; // anchor pos
		Anchor anchor; // anchor type
		TextStyle style;
		std::string text;
		std::uint32_t rgba;
		std::uint32_t outline_rgba = 0; // 0: no outline
		float outline_thickness = 1.f;
	};

	struct LineDraw {
		float x0, y0, x1, y1;
		std::uint32_t rgba;
	};

	struct QuadDraw {
		float x, y, w, h;
		std::uint32_t rgba;
	};

	class IRenderer {
	public:
		virtual ~IRenderer() = default;

		// Called once per frame before SubmitText so font sizes scale with window height.
		virtual void SetWindowSize(float win_w, float win_h) = 0;

		virtual void BeginFrame() = 0;
		virtual void Clear(std::uint8_t r, std::uint8_t g, std::uint8_t b) = 0;
		virtual void EndFrame() = 0;
		virtual void SubmitText(const TextDraw& draw) = 0;
		virtual void SubmitLine(const LineDraw& draw) = 0;
		virtual void SubmitQuad(const QuadDraw& draw) = 0;
		virtual void SubmitSprite(float x, float y, float w, float h, int texture_handle, float alpha = 1.f) = 0;
		virtual float MeasureTextWidth(std::string_view text, TextStyle style) = 0;
		virtual int LoadTexture(const std::string& path) = 0;
		virtual int LoadTextureAsync(const std::string& path) = 0;
		virtual void PollAsyncLoads() = 0;
		virtual bool IsTextureReady(int handle) = 0;
		virtual bool GetTextureSize(int handle, float& w, float& h) = 0;
	};
}