#pragma once

#include "GameplaySession.h"
#include "ReplayRecord.h"

#include <cstdint>
#include <optional>
#include "JudgeCommandBuffer.h"

namespace rfs {
	class RecordingSession {
	public:
		explicit RecordingSession(FrozenChart chart, GameplaySessionConfig config, ReplayRecord& replay);
		std::optional<TapCommandBuffer> HandleLaneTap(int lane, std::int32_t input_ms, std::int32_t song_offset_ms = 0);
		MissCommandBuffer Update(std::int32_t song_time_ms, std::int32_t song_offset_ms = 0);
		const GameplaySession& Gameplay() const noexcept { return gameplay_; }

	private:
		GameplaySession gameplay_;
		ReplayRecord& record_;
	};
}