#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau
{
	struct ModManifest
	{
		std::string name;
		std::string version;
		std::string author;
		std::string description;
		std::filesystem::path root;

		[[nodiscard]] std::filesystem::path scripts_root() const { return root / "scripts"; }
	};

	using ModManifestPtr = std::shared_ptr<const ModManifest>;
}
