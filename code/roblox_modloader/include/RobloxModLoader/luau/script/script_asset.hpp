#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau
{
	struct ScriptAsset
	{
		std::filesystem::path path;
		std::filesystem::file_time_type mtime{};

		[[nodiscard]] std::string chunk_name() const { return path.filename().string(); }
	};

	[[nodiscard]] bool is_script_file(const std::filesystem::path& path);

	[[nodiscard]] std::expected<std::string, std::string> read_source(const std::filesystem::path& path);

	[[nodiscard]] std::optional<std::filesystem::file_time_type> file_mtime(const std::filesystem::path& path) noexcept;
}
