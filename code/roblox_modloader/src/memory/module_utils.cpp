#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/module.hpp"
#include "RobloxModLoader/memory/module_utils.hpp"
#include "RobloxModLoader/platform/memory/host_image.hpp"

namespace rml::memory::module_utils {
    std::string get_module_name_from_address(const uintptr_t address) {
        const auto module_path = platform::module_path_containing(reinterpret_cast<const void *>(address));

        if (module_path.empty()) {
            return "Unknown";
        }

        return module_path.filename().string();
    }

    uintptr_t get_module_base_address(const std::string &module_name) {
        const memory::module target{std::string_view(module_name)};
        return target.loaded() ? target.begin().as<std::uintptr_t>() : 0;
    }

    uintptr_t get_roblox_studio_base() {
        return get_module_base_address(std::string(platform::studio_image_name()));
    }

    uintptr_t get_roblox_studio_rebased_address(const uintptr_t address, uintptr_t studio_base) {
        if (studio_base == 0) {
            studio_base = get_roblox_studio_base();
        }

        if (studio_base == 0) {
            return 0;
        }

        return address - studio_base + platform::studio_preferred_image_base();
    }
}
