#pragma once

#include "FrameContext.h"

namespace rfs {
	class IGameState {
	public:
		virtual ~IGameState() = default; // destructor -> Big Five Rule :D
		virtual void Update(const FrameContext& ctx) = 0; // pure virtual, perfect for interfaces/abstract classes
		virtual void Render() = 0;
	};
}