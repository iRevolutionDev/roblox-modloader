#pragma once
#include "RobloxModLoader/common.hpp"

namespace memory::module_utils {
    /**
     * @brief Get the module name from a given memory address
     * @param address Memory address to get module name from
     * @return Module name (filename only) or "Unknown" if not found
     */
    std::string get_module_name_from_address(uintptr_t address);

    /**
     * @brief Get the base address of a module by name
     * @param module_name Name of the module (e.g., "RobloxStudioBeta.exe")
     * @return Base address of the module or 0 if not found
     */
    uintptr_t get_module_base_address(const std::string& module_name);

    /**
     * @brief Get the base address of RobloxStudioBeta.exe
     * @return Base address of Roblox Studio or 0 if not found
     */
    uintptr_t get_roblox_studio_base();

    /**
     * @brief Calculate rebased address for Roblox Studio
     * @param address Original address
     * @param studio_base Base address of Roblox Studio (optional, will be retrieved if not provided)
     * @return Rebased address with default base 0x140000000 or 0 if studio not found
     */
    uintptr_t get_roblox_studio_rebased_address(uintptr_t address, uintptr_t studio_base = 0);
}
