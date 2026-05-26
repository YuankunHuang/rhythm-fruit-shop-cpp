#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace rfs {

	struct SongEntry {
		std::string id;
		std::string title;
		std::string audio_path;
		std::string chart_path;
		std::vector<std::string> difficulties;
	};

	class ChartCatalog {
	public:
		static ChartCatalog Load(const std::filesystem::path& catalog_path, std::string& out_error);

		bool IsValid() const noexcept { return valid_; }
		const std::vector<SongEntry>& Songs() const noexcept { return songs_; }

	private:
		bool valid_ = false;
		std::vector<SongEntry> songs_;
	};

}
