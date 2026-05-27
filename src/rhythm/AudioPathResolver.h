#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace rfs {

	class AudioPathResolver {
	public:
		static std::optional<std::filesystem::path> Resolve(
			const std::string& song_id,
			const std::string& hint_path = {});

		static std::string SongIdFromChartPath(const std::filesystem::path& chart_path);
	};

}
