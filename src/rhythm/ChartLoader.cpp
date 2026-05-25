#include "ChartLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace rfs {

std::optional<FrozenChart> ChartLoader::Load(const std::filesystem::path& path, LoadError& out_error) {

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

	// Resolve which sub-chart key to read.
	// Real charts: { "type": "service", "charts": { "service": [...] } }
	// Track charts: { "charts": { "easy": [...], "normal": [...] } }
	// Fall back to the first available key if type is absent or not matched.
	if (!j.contains("charts") || !j["charts"].is_object()) {
		out_error = { "chart_missing", "No 'charts' object found" };
		return std::nullopt;
	}
	const auto& charts_obj = j["charts"];

	std::string chart_key = j.value("type", "");
	if (chart_key.empty() || !charts_obj.contains(chart_key)) {
		// Fall back: pick the first available difficulty/type key
		auto it = charts_obj.begin();
		if (it == charts_obj.end()) {
			out_error = { "chart_empty", "charts object has no entries" };
			return std::nullopt;
		}
		chart_key = it.key();
	}

	const auto& note_arr = charts_obj[chart_key];
	if (!note_arr.is_array()) {
		out_error = { "chart_not_array", "charts." + chart_key + " is not an array" };
		return std::nullopt;
	}

	FrozenChart chart;
	chart.title_ = j.value("title", "Untitled");
	// Real charts don't store approachTimeMs — keep the sensible default (1600 ms).

	uint8_t max_lane = 0;
	for (const auto& n : note_arr) {
		int lane = n.value("lane", -1);
		if (lane < 0) continue;    // skip special/merged lanes

		NoteDef note{};
		note.id        = n.value("id", 0u);
		note.time_ms   = static_cast<int32_t>(n.value("time", 0.0) * 1000.0);
		note.lane      = static_cast<uint8_t>(lane);
		note.visual_id = static_cast<uint16_t>(n.value("fruit", 0u));
		if (note.lane > max_lane) max_lane = note.lane;
		chart.notes_.push_back(note);
	}

	chart.lane_count_ = static_cast<uint8_t>(max_lane + 1);
	return chart;
}

} // namespace rfs
