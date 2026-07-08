#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <filesystem>
#include <string_view>

namespace rml::mod
{
	class RML_EXPORT ModPaths
	{
	public:
		ModPaths() = default;
		explicit ModPaths(std::filesystem::path root);

		[[nodiscard]] bool valid() const noexcept;
		[[nodiscard]] const std::filesystem::path& root() const noexcept;
		[[nodiscard]] std::filesystem::path config() const;
		[[nodiscard]] std::filesystem::path storage() const;
		[[nodiscard]] std::filesystem::path dir(std::string_view name) const;
		[[nodiscard]] std::filesystem::path config_file(std::string_view filename) const;
		[[nodiscard]] std::filesystem::path file(std::string_view relative) const;

	private:
		static std::filesystem::path ensure(const std::filesystem::path& path);

		std::filesystem::path m_root;
	};
}
