#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCKAPI_

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

#include <spdlog/spdlog.h>
#include <MinHook.h>

#include "pointers_internal.hpp"

typedef bool (*original_is_internal_t)();

static original_is_internal_t original_is_internal = nullptr;
static void *target_function = nullptr;

namespace mod::hooks {
    static bool is_internal() {
        if (!g_pointers_internal) {
            return original_is_internal();
        }

        if (g_pointers_internal->m_roblox_pointers.m_is_internal_flag == 0) {
            return original_is_internal();
        }

        *reinterpret_cast<bool *>(g_pointers_internal->m_roblox_pointers.m_is_internal_flag) = true;

        return original_is_internal();
    }
}

class internal_developer_mod final : public mod_base {
    std::shared_ptr<spdlog::logger> logger;
    std::shared_ptr<pointers_internal> pointers_instance;

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

        pointers_instance = std::make_shared<pointers_internal>();

        if (!pointers_instance->m_roblox_pointers.m_is_internal) {
            logger->error("Failed to find is_internal function pointer");
            return;
        }

        if (pointers_instance->m_roblox_pointers.m_is_internal_flag == 0) {
            logger->error("Failed to find is_internal_flag pointer");
            return;
        }

        if (MH_Initialize() != MH_OK) {
            logger->error("Failed to initialize MinHook");
            return;
        }

        target_function = pointers_instance->m_roblox_pointers.m_is_internal;

        if (MH_CreateHook(target_function, &mod::hooks::is_internal,
                          reinterpret_cast<LPVOID *>(&original_is_internal)) != MH_OK) {
            logger->error("Failed to create hook for is_internal");
            MH_Uninitialize();
            return;
        }

        if (MH_EnableHook(target_function) != MH_OK) {
            logger->error("Failed to enable hook for is_internal");
            MH_Uninitialize();
            return;
        }

        logger->info("Successfully hooked is_internal function");
    }

    void on_unload() {
        logger->info("Internal Developer Mod unloading");

        if (target_function) {
            MH_DisableHook(target_function);
            MH_RemoveHook(target_function);
        }

        MH_Uninitialize();

        pointers_instance.reset();

        logger->info("Internal Developer Mod unloaded successfully");
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
