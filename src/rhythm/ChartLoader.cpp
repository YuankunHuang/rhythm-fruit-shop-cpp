#include "ChartLoader.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace rfs {

	std::optional<FrozenChart> ChartLoader::Load(const std::filesystem::path& path, LoadError& outError) {

		// load: create ifream
		std::ifstream file(path);
		if (!file) {
			outError = { "file_not_found", "Cannot open: " + path.string() };
			return std::nullopt;
		}

		// load: parse json
		nlohmann::json j;
		try {
			file >> j;
		}
		catch (const std::exception& e) {
			outError = { "json_parse_error", e.what() };
			return std::nullopt;
		}

		// validate: check schema version
		if (j.value("schemaVersion", 0) != 1) {
			outError = { "schema_mismatch", "Expected schemaVersion == 1" };
			return std::nullopt;
		}

		// validate: check required fields
		if (!j.contains("notes") || !j["notes"].is_array()) {
			outError = { "chart_notes_missing", "Missing or invalid 'notes' array" };
			return std::nullopt;
		}

		FrozenChart chart;
		chart.title_ = j.value("title", "Untitled");
		chart.approachTimeMs_ = j.value("approachTimeMs", 1600);
		chart.laneCount_ = static_cast<uint8_t>(j.value("laneCount", 4));

		for (const auto& noteJson : j["notes"]) {
			NoteDef note{};
			note.id = noteJson.value("id", 0u);
			note.timeMs = noteJson.value("timeMs", 0);
			note.lane = static_cast<uint8_t>(noteJson.value("lane", 0u));
			note.visualId = static_cast<uint16_t>(noteJson.value("visualId", 0u));
			chart.notes_.push_back(note);
		}

		return chart;
	}

}
