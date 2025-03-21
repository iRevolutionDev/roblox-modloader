#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"

// Define your hook functions in a namespace
namespace mod::hooks {
    static uint64_t* example_hook(uint64_t* instance, uint64_t param1, uint64_t param2) {
        // Your hook logic here before the original call
        
        // Call the original function
        auto result = hooking::get_original<&example_hook>()(instance, param1, param2);
        
        // Your hook logic here after the original call
        
        return result;
    }
}

class basic_mod final : public mod_base {
    std::shared_ptr<spdlog::logger> mod_logger;

public:
    basic_mod() {
        name = "Basic Mod";
        version = "1.0.0";
        author = "Your Name";
        description = "A basic mod example";
        mod_logger = logger::get_logger("ExampleMod");
    }

    void on_load() override {
        mod_logger->info("Mod loaded successfully!");

        // Register your hooks using the mod loader's hooking system
        // The hooks will be automatically managed by the mod loader
        hooking::detour_hook_helper::add<&mod::hooks::example_hook>(
            "EXAMPLE_HOOK",
            reinterpret_cast<void*>(GetModuleHandleA(nullptr)) // Replace with actual target address
        );
    }

    void on_attach() const {
        // Now you can use your logger
        mod_logger->info("Mod attached successfully!");
        mod_logger->debug("This is a debug message");
        mod_logger->warn("This is a warning message");
    }

    void on_detach() const {
        mod_logger->info("Mod is being detached");
    }
};

#define BASIC_MOD_API __declspec(dllexport)

extern "C" {
BASIC_MOD_API mod_base *start_mod() {
    return new basic_mod();
}

BASIC_MOD_API void uninstall_mod(const mod_base *mod) {
    delete mod;
}
}
