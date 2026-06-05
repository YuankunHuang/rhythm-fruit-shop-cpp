#pragma once

#include <cstddef>
#include "JudgeCommand.h"
#include <array>
#include <span>

namespace rfs {
	template<std::size_t Capacity>
	struct StaticCommandBuffer {
		std::array<JudgeCommand, Capacity> data{};
		std::size_t count = 0;

		void Push(JudgeCommand cmd) {
			if (count < Capacity) {
				data[count++] = cmd;
			}
		}

		std::span<const JudgeCommand> Span() const {
			return { data.data(), count };
		}
		
		void Clear() {
			count = 0;
		}
	};

	using TapCommandBuffer = StaticCommandBuffer<8>;
	using MissCommandBuffer = StaticCommandBuffer<32>;
}