#include "file.hpp"

#include "RobloxModLoader/internal/common.hpp"

namespace rml::filesystem
{
	File::File(std::filesystem::path path) :
	    m_path(std::move(path))
	{
	}

	const std::filesystem::path& File::path() const noexcept
	{
		return m_path;
	}

	bool File::exists() const
	{
		std::error_code ec;
		return std::filesystem::exists(m_path, ec);
	}

	std::filesystem::file_time_type File::last_write_time() const
	{
		std::error_code ec;
		const auto time = std::filesystem::last_write_time(m_path, ec);
		return ec ? std::filesystem::file_time_type{} : time;
	}

	std::expected<std::string, std::error_code> File::read_text() const
	{
		errno = 0;

		std::ifstream stream(m_path, std::ios::binary);
		if (!stream.is_open())
			return std::unexpected(last_io_error());

		std::string contents;
		stream.seekg(0, std::ios::end);
		if (const auto size = stream.tellg(); size > 0)
			contents.reserve(size);
		stream.seekg(0, std::ios::beg);

		contents.assign(std::istreambuf_iterator(stream), std::istreambuf_iterator<char>());
		if (stream.bad())
			return std::unexpected(last_io_error());

		return contents;
	}

	std::expected<toml::table, std::string> File::read_toml() const
	{
		const auto contents = read_text();
		if (!contents)
			return std::unexpected(contents.error().message());

		auto result = toml::parse(*contents, m_path.string());
		if (!result)
		{
			const auto& error = result.error();
			return std::unexpected(std::format("{} (line {}, column {})",
			    error.description(),
			    error.source().begin.line,
			    error.source().begin.column));
		}

		return std::move(result).table();
	}

	std::expected<void, std::error_code> File::write_text(const std::string_view contents) const
	{
		if (const auto parent = m_path.parent_path(); !parent.empty())
		{
			std::error_code ec;
			std::filesystem::create_directories(parent, ec);
		}

		errno = 0;

		std::ofstream stream(m_path, std::ios::binary | std::ios::trunc);
		if (!stream.is_open())
			return std::unexpected(last_io_error());

		stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
		stream.flush();
		if (!stream.good())
			return std::unexpected(last_io_error());

		return {};
	}

	std::expected<void, std::error_code> File::move_to(const std::filesystem::path& destination) const
	{
		std::error_code ec;
		std::filesystem::rename(m_path, destination, ec);
		if (ec)
			return std::unexpected(ec);

		return {};
	}

	std::error_code File::last_io_error()
	{
		const int code = errno;
		return std::error_code(code != 0 ? code : EIO, std::generic_category());
	}
}
