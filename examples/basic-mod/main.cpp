#include <RobloxModLoader/hooking/hooking.hpp>
#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/mod/mod_base.hpp>
#include <spdlog/spdlog.h>

// Define your hook functions in a namespace
namespace mod::hooks {
    static uint64_t *example_hook(uint64_t *instance, uint64_t param1, uint64_t param2) {
        // Your hook logic here before the original call

        // Call the original function
        auto result = rml::Hooking::get_original<&example_hook>()(instance, param1, param2);

        // Your hook logic here after the original call

        return result;
    }
}

class basic_mod final : public ModBase {
    std::shared_ptr<spdlog::logger> mod_logger;

public:
    basic_mod() {
        name = "Basic Mod";
        version = "1.0.0";
        author = "Your Name";
        description = "A basic mod example using RobloxModLoader";
        mod_logger = rml::Logger::get_logger("BasicMod");
    }

    void on_load() override {
        mod_logger->info("Basic Mod loaded successfully!");

        // Register your hooks using the mod loader's hooking system
        // The hooks will be automatically managed by the mod loader
        // hooking::detour_hook_helper::add_lazy<&mod::hooks::example_hook>(
        //     "EXAMPLE_HOOK",
        //     target_address  // Replace with actual target address from g_pointers
        // );
    }

    void on_unload() override {
        mod_logger->info("Basic Mod is being unloaded");
    }
};

#define BASIC_MOD_API __declspec(dllexport)

extern "C" {
BASIC_MOD_API ModBase *start_mod() {
    return new basic_mod();
}

BASIC_MOD_API void uninstall_mod(const ModBase *mod) {
    delete mod;
}
}

RML_EXPORT_MOD_ABI_VERSION()
