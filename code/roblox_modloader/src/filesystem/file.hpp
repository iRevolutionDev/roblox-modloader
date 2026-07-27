#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <toml++/toml.hpp>

namespace rml::filesystem
{
	class File
	{
	public:
		explicit File(std::filesystem::path path);

		[[nodiscard]] const std::filesystem::path& path() const noexcept;
		[[nodiscard]] bool exists() const;
		[[nodiscard]] std::filesystem::file_time_type last_write_time() const;

		[[nodiscard]] std::expected<std::string, std::error_code> read_text() const;
		[[nodiscard]] std::expected<toml::table, std::string> read_toml() const;
		[[nodiscard]] std::expected<void, std::error_code> write_text(std::string_view contents) const;
		[[nodiscard]] std::expected<void, std::error_code> move_to(const std::filesystem::path& destination) const;

	private:
		[[nodiscard]] static std::error_code last_io_error();

		std::filesystem::path m_path;
	};
}
