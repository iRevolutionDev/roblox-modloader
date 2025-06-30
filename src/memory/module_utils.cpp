#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/memory/module_utils.hpp"

namespace memory::module_utils {
    std::string get_module_name_from_address(const uintptr_t address) {
        HMODULE module_handle;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCSTR>(address), &module_handle)) {
            char module_name[MAX_PATH];
            if (GetModuleFileNameA(module_handle, module_name, MAX_PATH)) {
                const std::filesystem::path path(module_name);
                return path.filename().string();
            }
        }
        return "Unknown";
    }

    uintptr_t get_module_base_address(const std::string &module_name) {
        const auto module_handle = GetModuleHandleA(module_name.c_str());
        return reinterpret_cast<uintptr_t>(module_handle);
    }

    uintptr_t get_roblox_studio_base() {
        return get_module_base_address("RobloxStudioBeta.exe");
    }

    uintptr_t get_roblox_studio_rebased_address(const uintptr_t address, uintptr_t studio_base) {
        if (studio_base == 0) {
            studio_base = get_roblox_studio_base();
        }

        if (studio_base == 0) {
            return 0;
        }

        return address - studio_base + 0x140000000;
    }
}
