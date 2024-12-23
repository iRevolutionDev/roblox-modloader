#include <string>
#include <memory>
#include <iostream>
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"

#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/mod/mod.hpp"

class basic_mod final : public mod {
public:
    basic_mod() {
        name = "Basic Mod";
        version = "1.0.0";
        author = "Your Name";
        description = "A basic mod for Roblox Mod.";
    }

    ~basic_mod() override = default;

    void on_load() override {
        LOG_INFO("LOADED: {}", name);
    }
};

#define BASIC_MOD_API __declspec(dllexport)

extern "C" {
BASIC_MOD_API mod *start_mod() {
    return new basic_mod();
}

BASIC_MOD_API void uninstall_mod(const mod *mod) {
    delete mod;
}
}
