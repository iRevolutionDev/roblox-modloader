#include "directory.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/platform/memory/host_image.hpp"

namespace rml::utils
{
	std::filesystem::path directory::get_module_directory()
	{
		const auto module_path = platform::module_path_containing(reinterpret_cast<const void*>(&get_module_directory));

		if (module_path.empty())
		{
			std::cerr << "Failed to resolve the loader module path" << std::endl;
			return std::filesystem::current_path();
		}

		return module_path.parent_path();
	}

	std::filesystem::path directory::get_executable_directory()
	{
		const auto exe_path = platform::executable_path();

		if (exe_path.empty())
		{
			std::cerr << "Failed to resolve the executable path" << std::endl;
			return std::filesystem::current_path();
		}

		return exe_path.parent_path();
	}

	std::filesystem::path directory::get_mod_loader_directory()
	{
		return get_executable_directory() / "RobloxModLoader";
	}

	std::filesystem::path directory::get_runtime_directory()
	{
		return get_mod_loader_directory() / "runtime";
	}
}
