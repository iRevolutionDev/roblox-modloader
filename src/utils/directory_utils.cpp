#include "RobloxModLoader/common.hpp"
#include "directory_utils.hpp"

std::filesystem::path directory_utils::get_module_directory() {
    HMODULE hModule = nullptr;

    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&get_module_directory), &hModule)) {
        std::cerr << "Failed to get module handle: " << GetLastError() << std::endl;
        return std::filesystem::current_path();
    }

    wchar_t module_path[MAX_PATH];
    if (GetModuleFileNameW(hModule, module_path, MAX_PATH) == 0) {
        std::cerr << "Failed to get module filename: " << GetLastError() << std::endl;
        return std::filesystem::current_path();
    }

    std::filesystem::path module_dir = std::filesystem::path(module_path).parent_path();
    return module_dir;
}
