#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCKAPI_

#include <spdlog/spdlog.h>
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/memory/module.hpp"
#include "RobloxModLoader/memory/pattern.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include "RobloxModLoader/hooking/detour_hook.hpp"

namespace mod::hooks {
    static bool is_internal() {
        return true;
    }
}

class internal_developer_mod final : public mod_base {
    std::shared_ptr<spdlog::logger> logger;

public:
    internal_developer_mod() {
        name = "Internal Developer Mod";
        version = "1.0.0";
        author = "Revolution";
        description = "This mod enables developer features for Roblox Studio.";

        logger = logger::get_logger("InternalDeveloperMod");
    }

    void on_load() override {
        logger->info("Internal Developer Mod loaded");

        // hooking::detour_hook_helper::add<&mod::hooks::is_internal>("IS_INTERNAL",
        //                                                            reinterpret_cast<void *>(
        //                                                                reinterpret_cast<uintptr_t>(GetModuleHandleA(
        //                                                                    nullptr)) + 0x40EA280));
    }
};

#define INTERNAL_DEVELOPER_MOD_API __declspec(dllexport)

extern "C" {
INTERNAL_DEVELOPER_MOD_API mod_base *start_mod() {
    return new internal_developer_mod();
}

INTERNAL_DEVELOPER_MOD_API void uninstall_mod(const mod_base *mod) {
    delete mod;
}
}
