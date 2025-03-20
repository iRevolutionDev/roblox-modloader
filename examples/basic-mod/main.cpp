#include <string>
#include <memory>

#include <spdlog/spdlog.h>

#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

class basic_mod final : public mod_base {
    std::shared_ptr<spdlog::logger> mod_logger;

public:
    basic_mod() {
        name = "Basic Mod";
        version = "1.0.0";
        author = "Your Name";
        description = "A basic mod example";

        // Create a logger with your mod name
        mod_logger = logger::get_logger("ExampleMod");
    }

    void on_load() {
        mod_logger->info("Mod loaded successfully!");
    };

    void on_attach() {
        // Now you can use your logger
        mod_logger->info("Mod attached successfully!");
        mod_logger->debug("This is a debug message");
        mod_logger->warn("This is a warning message");
    }

    void on_detach() {
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
