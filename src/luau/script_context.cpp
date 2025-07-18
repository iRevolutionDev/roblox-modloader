#include "RobloxModLoader/common.hpp"
#include "lstate.h"
#include "lobject.h"
#include "RobloxModLoader/luau/script_context.hpp"

#include "RobloxModLoader/luau/environment/environment.hpp"
#include "RobloxModLoader/roblox/luau/roblox_extra_space.hpp"
#include "RobloxModLoader/roblox/security/script_permissions.hpp"

namespace rml::luau {
    ScriptContext::ScriptContext(const Context context)
        : m_context(context) {
    }

    ScriptContext::~ScriptContext() noexcept {
        shutdown();
    }

    bool ScriptContext::initialize() noexcept {
        try {
            std::unique_lock lock(m_state_mutex);

            m_is_initialized.store(true, std::memory_order_release);

            // Setup all global providers
            environment::setup_lua_environment(m_context.L);

            LOG_INFO("Execution context initialized successfully");
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Exception during execution context initialization: {}", e.what());
            return false;
        }
    }

    void ScriptContext::shutdown() noexcept {
        try {
            std::unique_lock lock(m_state_mutex);

            if (!m_is_initialized.load(std::memory_order_acquire)) {
                return;
            }

            LOG_INFO("Shutting down execution context");

            m_is_initialized.store(false, std::memory_order_release);

            LOG_INFO("Execution context shutdown complete. Final memory usage: {} bytes",
                     m_memory_usage.load(std::memory_order_relaxed));
        } catch (const std::exception &e) {
            LOG_ERROR("Exception during execution context shutdown: {}", e.what());
        }
    }

    lua_State *ScriptContext::get_thread_state() const noexcept {
        return m_context.L;
    }

    void ScriptContext::set_thread_identity(const lua_State *L, const RBX::Security::Permissions identity,
                                            const std::uint64_t capabilities) noexcept {
        if (!L) {
            LOG_ERROR("Cannot set thread identity: Lua state is null");
            return;
        }

        auto *extra_space = static_cast<RBX::Luau::RobloxExtraSpace *>(L->userdata);
        if (!extra_space) {
            LOG_ERROR("Cannot set thread identity: Thread extra space is null");
            return;
        }

        extra_space->context.identity = identity;
        extra_space->capabilities = capabilities;

        LOG_INFO("Set thread identity to {} with capabilities 0x{:X}", static_cast<int>(identity), capabilities);
    }

    void ScriptContext::elevate_closure(const Closure *closure, const std::uint64_t capabilities) noexcept {
        if (!closure || closure->isC) {
            return;
        }

        auto *security = new std::uint64_t();
        *security = capabilities;

        set_proto(closure->l.p, security);
    }

    void ScriptContext::set_proto(Proto *proto, std::uint64_t *security) noexcept {
        if (!proto) {
            return;
        }

        proto->userdata = static_cast<void *>(security);

        if (proto->sizep <= 0 || !proto->p) return;

        for (int i = 0; i < proto->sizep; ++i) {
            if (proto->p[i]) {
                set_proto(proto->p[i], security);
            }
        }
    }

    std::size_t ScriptContext::get_memory_usage() const noexcept {
        return m_memory_usage.load(std::memory_order_relaxed);
    }

    bool ScriptContext::is_initialized() const noexcept {
        return m_is_initialized.load(std::memory_order_acquire);
    }
}
