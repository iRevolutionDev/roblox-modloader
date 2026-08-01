#include "RobloxModLoader/luau/script/script_asset.hpp"

namespace rml::luau
{
	bool is_script_file(const std::filesystem::path& path)
	{
		const auto extension = path.extension().string();
		return extension == ".lua" || extension == ".luau";
	}

	std::expected<std::string, std::string> read_source(const std::filesystem::path& path)
	{
		std::error_code error;
		const auto status = std::filesystem::status(path, error);
		if (error)
		{
			return std::unexpected(std::format("cannot stat '{}': {}", path.string(), error.message()));
		}

		if (!std::filesystem::is_regular_file(status))
		{
			return std::unexpected(std::format("not a regular file: '{}'", path.string()));
		}

		std::ifstream stream(path, std::ios::binary);
		if (!stream.is_open())
		{
			return std::unexpected(std::format("cannot open '{}'", path.string()));
		}

		std::string source;
		if (const auto size = std::filesystem::file_size(path, error); !error)
		{
			source.reserve(static_cast<std::size_t>(size));
		}

		source.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

		if (stream.bad())
		{
			return std::unexpected(std::format("failed while reading '{}'", path.string()));
		}

		return source;
	}

	std::optional<std::filesystem::file_time_type> file_mtime(const std::filesystem::path& path) noexcept
	{
		std::error_code error;
		const auto time = std::filesystem::last_write_time(path, error);
		if (error)
		{
			return std::nullopt;
		}

		return time;
	}
}
