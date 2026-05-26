#pragma once

namespace rfs {
	class IWindow {
	public:
		virtual ~IWindow() = default;
		virtual bool  IsOpen() const = 0;
		virtual void  Close() = 0;
		virtual float Width()  const = 0;
		virtual float Height() const = 0;
		virtual void OnResize(unsigned width, unsigned height) = 0;
	};
}