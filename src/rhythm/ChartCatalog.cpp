#include "ChartCatalog.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace rfs {

	ChartCatalog ChartCatalog::Load(const std::filesystem::path& catalog_path, std::string& out_error) {
		ChartCatalog catalog{};

		std::ifstream file(catalog_path);
		if (!file) {
			out_error = "Cannot open catalog: " + catalog_path.string();
			return catalog;
		}

		nlohmann::json j;
		try {
			file >> j;
		}
		catch (const std::exception& e) {
			out_error = std::string("JSON parse error: ") + e.what();
			return catalog;
		}

		if (!j.contains("songs") || !j["songs"].is_array()) {
			out_error = "Catalog missing 'songs' array";
			return catalog;
		}

		for (const auto& entry : j["songs"]) {
			SongEntry song{};
			song.id = entry.value("id", "");
			song.title = entry.value("title", "");
			song.audio_path = entry.value("audio", "");
			song.chart_path = entry.value("chart", "");
			song.cover_path = entry.value("cover", "");

			if (entry.contains("difficulties") && entry["difficulties"].is_array()) {
				for (const auto& d : entry["difficulties"]) {
					if (d.is_string()) {
						song.difficulties.push_back(d.get<std::string>());
					}
				}
			}

			if (song.id.empty() || song.chart_path.empty() || song.difficulties.empty()) {
				continue;
			}

			catalog.songs_.push_back(std::move(song));
		}

		catalog.valid_ = true;
		return catalog;
	}

}
