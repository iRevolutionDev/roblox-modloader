#include "RobloxModLoader/luau/env/mod_environment.hpp"

#include "filesystem/directory.hpp"

namespace rml::luau
{
	std::filesystem::path default_rml_libraries_root()
	{
		return rml::filesystem::directory::get_executable_directory() / "RobloxModLoader" / "libraries";
	}
}
