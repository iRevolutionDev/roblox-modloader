#include <string>
#include <memory>

#include <spdlog/spdlog.h>

#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

class basic_mod final : public mod_base {
public:
    basic_mod() {
        name = "Basic Mod";
        version = "1.0.0";
        author = "Your Name";
        description = "A basic mod for Roblox Mod.";
    }

    ~basic_mod() override {
    };

    void on_load() override {
        //LOG_INFO("LOADED: {}", name);
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
