#include "RobloxModLoader/luau/env/mod_environment.hpp"

#include "filesystem/directory.hpp"

namespace rml::luau
{
	std::filesystem::path default_rml_libraries_root()
	{
		return rml::filesystem::directory::get_executable_directory() / "RobloxModLoader" / "libraries";
	}

	const ScriptTree* ModEnvironment::tree_for(const std::string_view logical) const noexcept
	{
		if (logical.starts_with("@rml"))
		{
			return libraries.get();
		}

		return scripts();
	}

	const ScriptNode* ModEnvironment::node_for(const std::string_view logical) const noexcept
	{
		const auto* tree = tree_for(logical);
		return tree == nullptr ? nullptr : tree->find(logical);
	}
}
