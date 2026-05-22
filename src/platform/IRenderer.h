#pragma once

namespace rfs {
	class IRenderer {
	public:
		virtual ~IRenderer() = default;
		virtual void Clear() = 0;
		virtual void Display() = 0;
	};
}