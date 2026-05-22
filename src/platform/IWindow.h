#pragma once

namespace rfs {
	class IWindow {
	public:
		virtual ~IWindow() = default; // destructor -> Big Five Rule :D
		virtual void Show() = 0;
		virtual void Hide() = 0;
	};
}