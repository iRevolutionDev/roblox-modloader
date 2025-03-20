#include <string>
#include <memory>

#include <spdlog/spdlog.h>

#include "RobloxModLoader/logger/logger.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

class internal_developer_mod final : public mod_base {
public:
    internal_developer_mod() {
        name = "Internal Developer Mod";
        version = "1.0.0";
        author = "Revolution";
        description = "This mod enables developer features for Roblox Studio.";
    }

    ~internal_developer_mod() override {
    }

    void on_load() override {
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
