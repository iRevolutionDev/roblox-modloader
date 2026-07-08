#include "RobloxModLoader/luau/environment/environment_context.hpp"
#include "RobloxModLoader/luau/script_engine.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau::environment {
    EnvironmentContext::EnvironmentContext(std::shared_ptr<ScriptEngine> parentEngine)
        : m_parent_engine_(std::move(parentEngine)) {
        LOG_INFO("EnvironmentContext created for ScriptEngine: {}", static_cast<void*>(m_parent_engine_.get()));
    }

    EnvironmentContext::~EnvironmentContext() {
        destroy_context();
    }

    void EnvironmentContext::register_hook(Closure *target, const HookInformation &info) {
        if (m_is_destroyed_) return;

        std::unique_lock lock(m_mutex);
        m_function_hooks[target] = info;
        LOG_DEBUG("Registered hook for closure: {}", static_cast<void*>(target));
    }

    void EnvironmentContext::unregister_hook(Closure *target) {
        if (m_is_destroyed_) return;

        std::unique_lock lock(m_mutex);
        auto it = m_function_hooks.find(target);
        if (it != m_function_hooks.end()) {
            m_function_hooks.erase(it);
            LOG_DEBUG("Unregistered hook for closure: {}", static_cast<void*>(target));
        }
    }

    bool EnvironmentContext::is_hooked(Closure *target) const {
        if (m_is_destroyed_) return false;

        std::shared_lock lock(m_mutex);
        return m_function_hooks.contains(target);
    }

    std::optional<HookInformation> EnvironmentContext::get_hook(Closure *target) const {
        if (m_is_destroyed_) return std::nullopt;

        std::shared_lock lock(m_mutex);
        auto it = m_function_hooks.find(target);
        if (it != m_function_hooks.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void EnvironmentContext::register_new_c_closure(Closure *wrapper,
                                                    const ReferencedLuauObject<Closure *, LUA_TFUNCTION> &original) {
        if (m_is_destroyed_) return;

        std::unique_lock lock(m_mutex);
        m_newcclosures[wrapper] = original;
        LOG_DEBUG("Registered newcclosure wrapper: {}", static_cast<void*>(wrapper));
    }

    void EnvironmentContext::unregister_new_c_closure(Closure *wrapper) {
        if (m_is_destroyed_) return;

        std::unique_lock lock(m_mutex);
        auto it = m_newcclosures.find(wrapper);
        if (it != m_newcclosures.end()) {
            m_newcclosures.erase(it);
            LOG_DEBUG("Unregistered newcclosure wrapper: {}", static_cast<void*>(wrapper));
        }
    }

    bool EnvironmentContext::is_wrapped_closure(Closure *wrapper) const {
        if (m_is_destroyed_) return false;

        std::shared_lock lock(m_mutex);
        return m_newcclosures.contains(wrapper);
    }

    std::optional<ReferencedLuauObject<Closure *, LUA_TFUNCTION> > EnvironmentContext::get_original(
        Closure *wrapper) const {
        if (m_is_destroyed_) return std::nullopt;

        std::shared_lock lock(m_mutex);
        if (const auto it = m_newcclosures.find(wrapper); it != m_newcclosures.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void EnvironmentContext::protect_function(Closure *closure) {
        if (m_is_destroyed_) return;

        std::unique_lock lock(m_mutex);
        m_protected_closures.insert(closure);
        LOG_DEBUG("Made protected closure: {}", static_cast<void*>(closure));
    }

    bool EnvironmentContext::is_protected(Closure *closure) const {
        if (m_is_destroyed_) return false;

        std::shared_lock lock(m_mutex);
        return m_protected_closures.contains(closure);
    }

    void EnvironmentContext::register_self_closure(Closure *closure) {
        if (m_is_destroyed_) return;

        std::unique_lock lock(m_mutex);
        m_our_closures.insert(closure);
        LOG_DEBUG("Registered our closure: {}", static_cast<void*>(closure));
    }

    bool EnvironmentContext::is_self_closure(Closure *closure) const {
        if (m_is_destroyed_) return false;

        std::shared_lock lock(m_mutex);
        return m_our_closures.contains(closure) || m_newcclosures.contains(closure);
    }

    void EnvironmentContext::destroy_context() {
        if (m_is_destroyed_.exchange(true)) {
            return;
        }

        LOG_DEBUG("Destroying EnvironmentContext...");

        std::unique_lock lock(m_mutex);
        m_function_hooks.clear();
        m_newcclosures.clear();
        m_protected_closures.clear();
        m_our_closures.clear();

        m_parent_engine_.reset();

        LOG_DEBUG("EnvironmentContext destroyed");
    }

    bool EnvironmentContext::is_destroyed() const {
        return m_is_destroyed_;
    }

    std::shared_ptr<ScriptEngine> EnvironmentContext::get_engine() const {
        if (m_is_destroyed_) return nullptr;
        return m_parent_engine_;
    }

    std::shared_ptr<EnvironmentContext> get_environment_context(lua_State *L) {
        if (!L || !rml::has_task_scheduler()) {
            LOG_ERROR("Invalid lua_State or task_scheduler not initialized");
            return nullptr;
        }

        try {
            const auto script_engine = rml::task_scheduler().get_script_engine(L);
            if (!script_engine) {
                LOG_ERROR("Failed to get script engine for lua_State: {}", static_cast<void*>(L));
                return nullptr;
            }

            auto environment_context = script_engine->get_environment_context();
            if (!environment_context) {
                LOG_WARN("No EnvironmentContext found for ScriptEngine");
                return nullptr;
            }

            return environment_context;
        } catch (const std::exception &e) {
            LOG_ERROR("Exception getting environment context: {}", e.what());
            return nullptr;
        }
    }
}
