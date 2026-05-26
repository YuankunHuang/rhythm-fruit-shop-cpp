#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <span>

namespace rfs {

	struct NoteDef final {
		std::uint32_t id = 0;
		std::int32_t time_ms = 0;
		std::uint8_t lane = 0;
		std::uint16_t visual_id = 0;
	};

	class FrozenChart final {
	public:
		const std::string& Title() const noexcept { return title_; }
		std::span<const NoteDef> Notes() const noexcept { return notes_; }
		std::int32_t ApproachTimeMs() const noexcept { return approach_time_ms_; }
		std::uint8_t LaneCount() const noexcept { return lane_count_; }

	private:
		friend class ChartLoader;
		std::string title_;
		std::int32_t approach_time_ms_ = 1600;
		std::uint8_t lane_count_ = 4;
		std::vector<NoteDef> notes_;
	};
}