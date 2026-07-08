#include "directory.hpp"

#include "RobloxModLoader/internal/common.hpp"

namespace rml::utils
{
	std::filesystem::path directory::get_module_directory()
	{
		HMODULE hModule = nullptr;

		if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&get_module_directory), &hModule))
		{
			std::cerr << "Failed to get module handle: " << GetLastError() << std::endl;
			return std::filesystem::current_path();
		}

		wchar_t module_path[MAX_PATH];
		if (GetModuleFileNameW(hModule, module_path, MAX_PATH) == 0)
		{
			std::cerr << "Failed to get module filename: " << GetLastError() << std::endl;
			return std::filesystem::current_path();
		}

		std::filesystem::path module_dir = std::filesystem::path(module_path).parent_path();
		return module_dir;
	}

	std::filesystem::path directory::get_executable_directory()
	{
		wchar_t exe_path[MAX_PATH];
		if (GetModuleFileNameW(nullptr, exe_path, MAX_PATH) == 0)
		{
			std::cerr << "Failed to get executable filename: " << GetLastError() << std::endl;
			return std::filesystem::current_path();
		}

		std::filesystem::path exe_dir = std::filesystem::path(exe_path).parent_path();
		return exe_dir;
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