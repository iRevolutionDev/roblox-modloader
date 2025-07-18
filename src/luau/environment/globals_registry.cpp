#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/luau/environment/globals_registry.hpp"

namespace rml::luau::environment {
    void LuaFunctionRegistry::register_function_safe(lua_State *L, const std::string_view name,
                                                     const lua_CFunction func) noexcept {
        lua_pushcfunction(L, func, name.data());
        lua_setglobal(L, name.data());
    }

    template<lua_CFunction Func>
    int LuaFunctionRegistry::safe_call_wrapper(lua_State *L) noexcept {
        try {
            return Func(L);
        } catch (const std::exception &e) {
            return handle_lua_error(L, "unknown", e);
        } catch (...) {
            return handle_lua_unknown_error(L, "unknown");
        }
    }

    int LuaFunctionRegistry::handle_lua_error(lua_State *L, std::string_view function_name,
                                              const std::exception &e) noexcept {
        const auto error_msg = std::format("Error in function '{}': {}", function_name, e.what());
        lua_pushstring(L, error_msg.c_str());
        lua_error(L);
    }

    int LuaFunctionRegistry::handle_lua_unknown_error(lua_State *L, std::string_view function_name) noexcept {
        const auto error_msg = std::format("Unknown error in function '{}'", function_name);
        lua_pushstring(L, error_msg.c_str());
        lua_error(L);
    }

    template<typename Derived>
    void GlobalProvider<Derived>::register_function(lua_State *L, const std::string_view name,
                                                    const lua_CFunction func) noexcept {
        LuaFunctionRegistry::register_function_safe(L, name, func);
    }

    template<typename Derived>
    void GlobalProvider<Derived>::register_table(lua_State *L, const std::string_view name) noexcept {
        lua_newtable(L);
        lua_setglobal(L, name.data());
    }

    template<typename Derived>
    void GlobalProvider<
        Derived>::register_string(lua_State *L, const std::string_view name, const std::string_view value) noexcept {
        lua_pushstring(L, value.data());
        lua_setglobal(L, name.data());
    }

    template<typename Derived>
    void GlobalProvider<Derived>::register_number(lua_State *L, const std::string_view name,
                                                  const lua_Number value) noexcept {
        lua_pushnumber(L, value);
        lua_setglobal(L, name.data());
    }

    template<typename Derived>
    void GlobalProvider<
        Derived>::register_boolean(lua_State *L, const std::string_view name, const bool value) noexcept {
        lua_pushboolean(L, value);
        lua_setglobal(L, name.data());
    }

    template<typename Derived>
    void GlobalProvider<Derived>::register_table_function(lua_State *L, const std::string_view name,
                                                          const lua_CFunction func) noexcept {
        lua_pushcfunction(L, func, name.data());
        lua_setfield(L, -2, name.data());
    }

    template<typename Derived>
    void GlobalProvider<Derived>::begin_table_registration(lua_State *L, std::string_view table_name) noexcept {
        lua_newtable(L);
    }

    template<typename Derived>
    void GlobalProvider<Derived>::end_table_registration(lua_State *L) noexcept {
    }

    class GlobalsRegistry::Impl {
    public:
        std::unordered_map<std::string, std::unique_ptr<IGlobalProvider> > providers;
        std::unordered_map<std::string, GlobalRegistryInfo> registry_info;
        mutable std::shared_mutex registry_mutex;
    };

    GlobalsRegistry &GlobalsRegistry::instance() noexcept {
        static GlobalsRegistry instance;
        return instance;
    }

    void GlobalsRegistry::ensure_impl() {
        if (!m_impl) {
            m_impl = std::make_unique<Impl>();
        }
    }

    std::expected<void, std::string> GlobalsRegistry::register_provider_impl(
        std::unique_ptr<IGlobalProvider> provider,
        const std::string &name) noexcept {
        ensure_impl();
        std::unique_lock lock(m_impl->registry_mutex);

        if (m_impl->providers.contains(name)) {
            return std::unexpected(std::format("Provider '{}' already registered", name));
        }

        provider->on_initialize();
        m_impl->registry_info.emplace(name, GlobalRegistryInfo(name));
        m_impl->providers.emplace(name, std::move(provider));

        LOG_INFO("Registered global provider: {}", name);
        return {};
    }

    std::expected<void, std::string> GlobalsRegistry::unregister_provider(std::string_view name) noexcept {
        ensure_impl();
        std::unique_lock lock(m_impl->registry_mutex);

        const auto name_str = std::string(name);
        const auto it = m_impl->providers.find(name_str);
        if (it == m_impl->providers.end()) {
            return std::unexpected(std::format("Provider '{}' not found", name));
        }

        it->second->on_shutdown();
        m_impl->providers.erase(it);
        m_impl->registry_info.erase(name_str);

        LOG_INFO("Unregistered global provider: {}", name);
        return {};
    }

    bool GlobalsRegistry::register_all_globals(lua_State *L) const noexcept {
        if (!m_impl) {
            return true;
        }

        std::shared_lock lock(m_impl->registry_mutex);

        bool success = true;
        for (const auto &[name, provider]: m_impl->providers) {
            try {
                if (!provider->register_globals(L)) {
                    LOG_ERROR("Failed to register globals for provider: {}", name);
                    success = false;
                }
            } catch (const std::exception &e) {
                LOG_ERROR("Exception while registering globals for provider '{}': {}", name, e.what());
                success = false;
            } catch (...) {
                LOG_ERROR("Unknown exception while registering globals for provider: {}", name);
                success = false;
            }
        }

        return success;
    }

    std::optional<GlobalRegistryInfo> GlobalsRegistry::get_provider_info(std::string_view name) const noexcept {
        if (!m_impl) {
            return std::nullopt;
        }

        std::shared_lock lock(m_impl->registry_mutex);

        const auto it = m_impl->registry_info.find(std::string(name));
        return it != m_impl->registry_info.end() ? std::optional{it->second} : std::nullopt;
    }

    std::vector<std::string> GlobalsRegistry::list_providers() const noexcept {
        if (!m_impl) {
            return {};
        }

        std::shared_lock lock(m_impl->registry_mutex);

        std::vector<std::string> providers;
        providers.reserve(m_impl->providers.size());

        for (const auto &name: m_impl->providers | std::views::keys) {
            providers.push_back(name);
        }

        return providers;
    }

    GlobalsRegistry::RegistryStats GlobalsRegistry::get_statistics() const noexcept {
        if (!m_impl) {
            return {};
        }

        std::shared_lock lock(m_impl->registry_mutex);

        RegistryStats stats;
        stats.total_providers = m_impl->providers.size();

        if (!m_impl->registry_info.empty()) {
            auto [min_it, max_it] = std::minmax_element(
                m_impl->registry_info.begin(), m_impl->registry_info.end(),
                [](const auto &a, const auto &b) {
                    return a.second.registration_time < b.second.registration_time;
                }
            );

            stats.oldest_registration = min_it->second.registration_time;
            stats.newest_registration = max_it->second.registration_time;
        }

        return stats;
    }

    void GlobalsRegistry::clear() const noexcept {
        if (!m_impl) {
            return;
        }

        std::unique_lock lock(m_impl->registry_mutex);

        for (const auto &provider: m_impl->providers | std::views::values) {
            try {
                provider->on_shutdown();
            } catch (...) {
            }
        }

        m_impl->providers.clear();
        m_impl->registry_info.clear();

        LOG_INFO("Cleared all global providers");
    }

    bool register_all_globals(lua_State *L) noexcept {
        return GlobalsRegistry::instance().register_all_globals(L);
    }
}
