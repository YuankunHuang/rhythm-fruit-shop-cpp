#include "RecordingSession.h"

namespace rfs {
	RecordingSession::RecordingSession(FrozenChart chart, GameplaySessionConfig config, ReplayRecord& record)
		: gameplay_(std::move(chart), config)
		, record_(record)
	{
		record_.config = config;
	}

	std::optional<TapCommandBuffer> RecordingSession::HandleLaneTap(int lane, std::int32_t input_ms) {
		record_.events.push_back(ReplayEvent{
			.lane = lane,
			.input_ms = input_ms,
			.kind = ReplayEventKind::Tap,
			});
		return gameplay_.HandleLaneTap(lane, input_ms);
	}

	MissCommandBuffer RecordingSession::Update(std::int32_t song_time_ms) {
		record_.events.push_back(ReplayEvent{
			.lane = -1,
			.input_ms = song_time_ms,
			.kind = ReplayEventKind::Update,
			});
		return gameplay_.Update(song_time_ms);
	}
}