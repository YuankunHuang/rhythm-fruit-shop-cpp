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
		std::optional<FrozenChart> Load(const std::filesystem::path& path, LoadError& out_error);
	};

}