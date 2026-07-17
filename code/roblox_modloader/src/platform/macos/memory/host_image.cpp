#include "RobloxModLoader/platform/memory/host_image.hpp"

#include <dlfcn.h>
#include <limits.h>
#include <mach-o/dyld.h>

namespace rml::platform
{
	std::filesystem::path module_path_containing(const void* address)
	{
		Dl_info info{};

		if (!dladdr(address, &info) || !info.dli_fname)
			return {};

		return info.dli_fname;
	}

	std::filesystem::path executable_path()
	{
		char exe_path[PATH_MAX];
		auto size = static_cast<std::uint32_t>(sizeof(exe_path));

		if (_NSGetExecutablePath(exe_path, &size) != 0)
			return {};

		return exe_path;
	}

	std::string_view studio_image_name()
	{
		return "RobloxStudio";
	}

	std::uintptr_t studio_preferred_image_base()
	{
		return 0x100000000;
	}

	void* acquire_main_window()
	{
		return nullptr;
	}
}
