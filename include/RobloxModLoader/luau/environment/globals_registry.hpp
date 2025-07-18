#pragma once

#include "RobloxModLoader/common.hpp"

namespace rml::luau::environment {
    template<typename T>
    concept LuaCallable = requires(T t, lua_State *L)
    {
        { t(L) } -> std::convertible_to<int>;
    };

    template<typename T>
    concept LuaGlobalProvider = requires(T t)
    {
        { T::NAME } -> std::convertible_to<std::string_view>;
        { t.register_globals(std::declval<lua_State *>()) } -> std::same_as<bool>;
    };

    struct GlobalRegistryInfo {
        std::string name;
        std::chrono::system_clock::time_point registration_time;

        GlobalRegistryInfo() = default;

        explicit GlobalRegistryInfo(std::string_view name)
            : name(name), registration_time(std::chrono::system_clock::now()) {
        }
    };

    class LuaFunctionRegistry {
    public:
        static void register_function_safe(lua_State *L, std::string_view name, lua_CFunction func) noexcept;

        template<lua_CFunction Func>
        static int safe_call_wrapper(lua_State *L) noexcept;

    private:
        static int handle_lua_error(lua_State *L, std::string_view function_name, const std::exception &e) noexcept;

        static int handle_lua_unknown_error(lua_State *L, std::string_view function_name) noexcept;
    };

    class IGlobalProvider {
    public:
        virtual ~IGlobalProvider() = default;

        virtual bool register_globals(lua_State *L) noexcept = 0;

        virtual void on_initialize() noexcept {
        }

        virtual void on_shutdown() noexcept {
        }
    };

    template<typename Derived>
    class GlobalProvider : public IGlobalProvider {
    public:
        ~GlobalProvider() override = default;

        bool register_globals(lua_State *L) noexcept override = 0;

        void on_initialize() noexcept override {
        }

        void on_shutdown() noexcept override {
        }

    protected:
        static void register_function(lua_State *L, std::string_view name, lua_CFunction func) noexcept;

        static void register_table(lua_State *L, std::string_view name) noexcept;

        static void register_string(lua_State *L, std::string_view name, std::string_view value) noexcept;

        static void register_number(lua_State *L, std::string_view name, lua_Number value) noexcept;

        static void register_boolean(lua_State *L, std::string_view name, bool value) noexcept;

        static void register_table_function(lua_State *L, std::string_view name, lua_CFunction func) noexcept;

        static void begin_table_registration(lua_State *L, std::string_view table_name) noexcept;

        static void end_table_registration(lua_State *L) noexcept;
    };

    class GlobalsRegistry final {
    public:
        struct RegistryStats {
            std::size_t total_providers{0};
            std::chrono::system_clock::time_point oldest_registration;
            std::chrono::system_clock::time_point newest_registration;
        };

        GlobalsRegistry(const GlobalsRegistry &) = delete;

        GlobalsRegistry &operator=(const GlobalsRegistry &) = delete;

        GlobalsRegistry(GlobalsRegistry &&) = delete;

        GlobalsRegistry &operator=(GlobalsRegistry &&) = delete;

        [[nodiscard]] static GlobalsRegistry &instance() noexcept;

        template<LuaGlobalProvider T>
        std::expected<void, std::string> register_provider() noexcept;

        std::expected<void, std::string> unregister_provider(std::string_view name) noexcept;

        bool register_all_globals(lua_State *L) const noexcept;

        void clear() const noexcept;

        [[nodiscard]] std::optional<GlobalRegistryInfo> get_provider_info(std::string_view name) const noexcept;

        [[nodiscard]] std::vector<std::string> list_providers() const noexcept;

        [[nodiscard]] RegistryStats get_statistics() const noexcept;

    private:
        GlobalsRegistry() = default;

        ~GlobalsRegistry() = default;

        class Impl;
        std::unique_ptr<Impl> m_impl;

        void ensure_impl();

        std::expected<void, std::string> register_provider_impl(
            std::unique_ptr<IGlobalProvider> provider, const std::string &name) noexcept;
    };

    bool register_all_globals(lua_State *L) noexcept;

    template<LuaGlobalProvider T>
    std::expected<void, std::string> GlobalsRegistry::register_provider() noexcept {
        ensure_impl();

        try {
            auto provider = std::make_unique<T>();
            const auto name = std::string(T::NAME);

            auto base_provider = std::unique_ptr<IGlobalProvider>(
                static_cast<IGlobalProvider *>(provider.release())
            );

            return register_provider_impl(std::move(base_provider), name);
        } catch (const std::exception &e) {
            return std::unexpected(std::format("Failed to register provider: {}", e.what()));
        }
    }
}

#define RML_REGISTER_GLOBAL_PROVIDER(ProviderType) \
    do { \
        if (auto result = rml::luau::environment::GlobalsRegistry::instance().register_provider<ProviderType>(); !result) { \
            LOG_ERROR("Failed to register global provider {}: {}", #ProviderType, result.error()); \
        } \
    } while(0)
