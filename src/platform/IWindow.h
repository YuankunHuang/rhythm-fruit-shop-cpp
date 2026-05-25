#pragma once

namespace rfs {
	class IWindow {
	public:
		virtual ~IWindow() = default;
		virtual bool IsOpen() const = 0;
		virtual void Close() = 0;
	};
}