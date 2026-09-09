#pragma once

#include "RobloxModLoader/luau/script/script_tree.hpp"

namespace rml::luau
{
	struct ModManifest
	{
		std::string name;
		std::string version;
		std::string author;
		std::string description;
		std::filesystem::path root;
		ScriptTreePtr scripts;

		[[nodiscard]] std::filesystem::path scripts_root() const { return root / "scripts"; }
	};

	using ModManifestPtr = std::shared_ptr<const ModManifest>;
}
