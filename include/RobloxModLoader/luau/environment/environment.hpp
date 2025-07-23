#pragma once

#include "RobloxModLoader/common.hpp"
#include "globals_registry.hpp"
#include "rml_provider.hpp"
#include "require_provider.hpp"
#include "bridge_provider.hpp"

namespace rml::luau::environment {
    inline void initialize_default_providers() noexcept {
        LOG_INFO("Initializing default global providers...");

        RML_REGISTER_GLOBAL_PROVIDER(RequireProvider);
        RML_REGISTER_GLOBAL_PROVIDER(RMLProvider);
        RML_REGISTER_GLOBAL_PROVIDER(BridgeProvider);

        const auto stats = GlobalsRegistry::instance().get_statistics();
        LOG_INFO("Initialized {} global providers", stats.total_providers);
    }

    inline void shutdown_providers() noexcept {
        LOG_INFO("Shutting down global providers...");
        GlobalsRegistry::instance().clear();
        LOG_INFO("Global providers shutdown complete");
    }

    inline bool setup_lua_environment(lua_State *L) noexcept {
        if (!L) {
            LOG_ERROR("Lua state is null");
            return false;
        }

        const auto old_top = lua_gettop(L);

        const bool success = register_all_globals(L);
        if (success) {
            LOG_DEBUG("Lua environment setup completed successfully");
        } else {
            LOG_ERROR("Failed to setup Lua environment");
        }

        lua_settop(L, old_top); // Reset the stack to its original state

        return success;
    }
}
