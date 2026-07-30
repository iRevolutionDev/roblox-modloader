#pragma once

#include "RobloxModLoader/luau/script/mod_manifest.hpp"

namespace rml::luau
{
	struct ModEnvironment
	{
		ModManifestPtr manifest;
		std::filesystem::path rml_libraries;

		[[nodiscard]] std::filesystem::path mod_scripts() const
		{
			return manifest ? manifest->scripts_root() : std::filesystem::path{};
		}

		[[nodiscard]] std::string_view mod_name() const noexcept
		{
			return manifest ? std::string_view{manifest->name} : std::string_view{"<loader>"};
		}
	};

	[[nodiscard]] std::filesystem::path default_rml_libraries_root();
}
