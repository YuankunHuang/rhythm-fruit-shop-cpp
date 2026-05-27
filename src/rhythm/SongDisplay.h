#pragma once

#include <string>
#include <string_view>

namespace rfs {

	struct SongEntry;

	std::string HumanizeSongId(std::string_view song_id);
	std::string DisplaySongTitle(const SongEntry& entry);

}
