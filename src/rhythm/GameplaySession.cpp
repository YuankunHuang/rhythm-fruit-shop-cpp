#include "GameplaySession.h"

namespace rfs {
	GameplaySession::GameplaySession(FrozenChart chart, GameplaySessionConfig config)
		: chart_(std::move(chart))
		, config_(std::move(config))
		, store_(chart_.Notes().size())
		, judge_(config_.judgement)
	{
		const auto& notes = chart_.Notes();
		const std::int32_t last = notes.empty() ? 0 : notes[notes.size() - 1].time_ms;
		gameplay_end_ms_ = last + judge_.Config().good_window_ms;
		chart_end_ms_ = gameplay_end_ms_ + config_.song_end_delay_ms;
	}

	void GameplaySession::CommitCommand(const JudgeCommand& cmd) noexcept {
		store_.Apply(cmd);
		score_.Apply(cmd);
	}

	std::optional<TapCommandBuffer> GameplaySession::HandleLaneTap(int lane, std::int32_t input_ms) {
		const auto taps = judge_.JudgeTaps(
			chart_, store_.NoteResolved(), store_.NextIdx(),
			lane, input_ms, config_.song_offset_ms);
		if (taps.count == 0) {
			return std::nullopt;
		}
		for (const auto& cmd : taps.Span()) {
			CommitCommand(cmd);
		}
		return taps;
	}

	MissCommandBuffer GameplaySession::Update(std::int32_t song_time_ms) {
		const auto misses = judge_.DetectMisses(
			chart_, store_.NoteResolved(), store_.NextIdx(),
			song_time_ms, config_.song_offset_ms);
		for (const auto& cmd : misses.Span()) {
			CommitCommand(cmd);
		}
		store_.AdvancePastMissWindow(
			chart_, song_time_ms, config_.song_offset_ms, config_.judgement.good_window_ms);
		return misses;
	}

	bool GameplaySession::IsFinished(std::int32_t song_time_ms) const noexcept {
		return song_time_ms >= chart_end_ms_
			&& store_.NextIdx() >= chart_.Notes().size();
	}

	GameResult GameplaySession::Summary() const noexcept {
		return score_.BuildResult();
	}
}
