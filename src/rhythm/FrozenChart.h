#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <span>

namespace rfs{

	struct NoteDef final {
		std::uint32_t id = 0; // unique identifier for this note, used for scoring and debugging
		std::int32_t timeMs = 0; // in milliseconds from the start of the song
		std::uint8_t lane = 0;
		std::uint16_t visualId = 0; // index into sprite sheet, for rendering
	};

	class FrozenChart final {
	public:
		const std::string& Title() const noexcept { return title_; }
		std::span<const NoteDef> Notes() const noexcept { return notes_; }
		std::int32_t ApproachTimeMs() const noexcept { return approachTimeMs_; }
		std::uint8_t LaneCount() const noexcept { return laneCount_; }

	private:
		friend class ChartLoader; // only ChartLoader can create instances of FrozenChart, since the constructor is private

		std::string title_;
		std::int32_t approachTimeMs_ = 1600;
		std::uint8_t laneCount_ = 4;
		std::vector<NoteDef> notes_;
	};
}