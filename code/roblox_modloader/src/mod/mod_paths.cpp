#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/mod/mod_paths.hpp"

#include <system_error>
#include <utility>

namespace rml::mod
{
	ModPaths::ModPaths(std::filesystem::path root)
		: m_root(std::move(root))
	{
	}

	bool ModPaths::valid() const noexcept
	{
		return !m_root.empty();
	}

	const std::filesystem::path& ModPaths::root() const noexcept
	{
		return m_root;
	}

	std::filesystem::path ModPaths::ensure(const std::filesystem::path& path)
	{
		std::error_code ec;
		std::filesystem::create_directories(path, ec);
		if (ec)
		{
			LOG_WARN("[ModPaths::ensure] Failed to create directory '{}': {}", path.string(), ec.message());
		}

		return path;
	}

	std::filesystem::path ModPaths::config() const
	{
		return ensure(m_root / "config");
	}

	std::filesystem::path ModPaths::storage() const
	{
		return ensure(m_root / "storage");
	}

	std::filesystem::path ModPaths::dir(const std::string_view name) const
	{
		return ensure(m_root / std::filesystem::path(name));
	}

	std::filesystem::path ModPaths::config_file(const std::string_view filename) const
	{
		return config() / std::filesystem::path(filename);
	}

	std::filesystem::path ModPaths::file(const std::string_view relative) const
	{
		return m_root / std::filesystem::path(relative);
	}
}
