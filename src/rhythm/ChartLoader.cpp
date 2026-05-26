#include "ChartLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace rfs {

	std::optional<FrozenChart> ChartLoader::Load(
		const std::filesystem::path& path,
		const std::string& difficulty,
		LoadError& out_error)
	{
		std::ifstream file(path);
		if (!file) {
			out_error = { "file_not_found", "Cannot open: " + path.string() };
			return std::nullopt;
		}

		nlohmann::json j;
		try {
			file >> j;
		}
		catch (const std::exception& e) {
			out_error = { "json_parse_error", e.what() };
			return std::nullopt;
		}

		if (j.value("schema", "") != "rfs-cpp-v1") {
			out_error = { "wrong_schema", "Expected schema 'rfs-cpp-v1'" };
			return std::nullopt;
		}

		if (!j.contains("difficulties") || !j["difficulties"].is_object()) {
			out_error = { "no_difficulties", "Missing 'difficulties' object" };
			return std::nullopt;
		}

		const auto& diffs = j["difficulties"];
		if (!diffs.contains(difficulty) || !diffs[difficulty].is_array()) {
			out_error = { "diff_not_found", "Difficulty '" + difficulty + "' not found" };
			return std::nullopt;
		}

		FrozenChart chart;
		chart.title_ = j.value("title", "Untitled");
		chart.approach_time_ms_ = j.value("approach_time_ms", 1600);

		const auto& note_arr = diffs[difficulty];
		uint8_t max_lane = 0;
		for (const auto& n : note_arr) {
			int lane = n.value("lane", -1);
			if (lane < 0 || lane > 3) continue;

			NoteDef note{};
			note.id = n.value("id", 0u);
			note.time_ms = n.value("time_ms", 0);
			note.lane = static_cast<uint8_t>(lane);
			note.visual_id = static_cast<uint16_t>(n.value("visual", 0u));
			if (note.lane > max_lane) max_lane = note.lane;
			chart.notes_.push_back(note);
		}

		chart.lane_count_ = chart.notes_.empty() ? 4 : static_cast<uint8_t>(max_lane + 1);
		return chart;
	}

}
