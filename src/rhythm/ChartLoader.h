#pragma once

#include <optional>
#include "FrozenChart.h"
#include <filesystem>
#include <string>

namespace rfs {

	struct LoadError {
		std::string code;
		std::string message;
	};

	class ChartLoader final {
	public:
		// Loads a chart in rfs-cpp-v1 format.
		// difficulty must match a key inside the "difficulties" object (e.g. "easy", "expert").
		std::optional<FrozenChart> Load(
			const std::filesystem::path& path,
			const std::string& difficulty,
			LoadError& out_error);
	};

}