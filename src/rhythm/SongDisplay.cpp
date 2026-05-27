#include "SongDisplay.h"
#include "ChartCatalog.h"
#include <cctype>

namespace rfs {

	std::string HumanizeSongId(std::string_view song_id) {
		std::string out;
		out.reserve(song_id.size());
		bool capitalize_next = true;

		for (char ch : song_id) {
			if (ch == '-' || ch == '_') {
				if (!out.empty() && out.back() != ' ') {
					out.push_back(' ');
				}
				capitalize_next = true;
				continue;
			}
			if (capitalize_next && std::isalpha(static_cast<unsigned char>(ch))) {
				out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
				capitalize_next = false;
			}
			else {
				out.push_back(ch);
				capitalize_next = false;
			}
		}
		return out;
	}

	std::string DisplaySongTitle(const SongEntry& entry) {
		if (entry.title.empty() || entry.title == "source" || entry.title == "Untitled") {
			return HumanizeSongId(entry.id);
		}
		return entry.title;
	}

}
