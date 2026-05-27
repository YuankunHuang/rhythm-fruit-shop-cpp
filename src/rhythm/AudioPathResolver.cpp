#include "AudioPathResolver.h"
#include <vector>

namespace rfs {

	namespace {
		const std::vector<std::string> kAudioExtensions = { ".mp3", ".m4a", ".ogg", ".wav" };
		const std::vector<std::string> kAudioSubdirs = { "", "tracks", "service" };

		std::string UnderscoreVariant(std::string id) {
			for (char& c : id) {
				if (c == '-') c = '_';
			}
			return id;
		}

		bool FileExists(const std::filesystem::path& path) {
			return !path.empty() && std::filesystem::exists(path);
		}
	}

	std::string AudioPathResolver::SongIdFromChartPath(const std::filesystem::path& chart_path) {
		auto stem = chart_path.stem().string();
		if (stem.size() >= 4 && stem.substr(stem.size() - 4) == ".rfs") {
			stem = stem.substr(0, stem.size() - 4);
		}
		return stem;
	}

	std::optional<std::filesystem::path> AudioPathResolver::Resolve(
		const std::string& song_id,
		const std::string& hint_path)
	{
		if (!hint_path.empty() && FileExists(hint_path)) {
			return std::filesystem::path(hint_path);
		}

		const std::vector<std::string> names = { song_id, UnderscoreVariant(song_id) };
		const std::filesystem::path audio_root = "assets/audio";

		for (const auto& sub : kAudioSubdirs) {
			for (const auto& name : names) {
				for (const auto& ext : kAudioExtensions) {
					std::filesystem::path candidate = audio_root;
					if (!sub.empty()) {
						candidate /= sub;
					}
					candidate /= (name + ext);
					if (FileExists(candidate)) {
						return candidate;
					}
				}
			}
		}

		return std::nullopt;
	}

}
