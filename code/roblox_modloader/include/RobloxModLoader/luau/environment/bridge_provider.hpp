#pragma once

#include "RobloxModLoader/common.hpp"
#include "globals_registry.hpp"
#include <functional>
#include <unordered_map>
#include <variant>
#include <memory>
#include <shared_mutex>

namespace rml::luau::environment {
    class LuaTable {
    public:
        LuaTable() = default;

        template<typename T>
        T get(const std::string &key, const T &default_value = T{}) const {
            auto it = data_.find(key);
            if (it == data_.end()) {
                return default_value;
            }

            if constexpr (std::is_same_v<T, std::string>) {
                if (const auto *str_val = std::get_if<std::string>(&it->second)) {
                    return *str_val;
                }
            } else if constexpr (std::is_same_v<T, double>) {
                if (const auto *num_val = std::get_if<double>(&it->second)) {
                    return *num_val;
                }
            } else if constexpr (std::is_same_v<T, bool>) {
                if (const auto *bool_val = std::get_if<bool>(&it->second)) {
                    return *bool_val;
                }
            }

            return default_value;
        }

        bool has_key(const std::string &key) const {
            return data_.find(key) != data_.end();
        }

        void set(const std::string &key, const std::variant<std::string, double, bool, std::nullptr_t> &value) {
            data_[key] = value;
        }

        size_t size() const { return data_.size(); }
        bool empty() const { return data_.empty(); }

        auto begin() const { return data_.begin(); }
        auto end() const { return data_.end(); }

    private:
        std::unordered_map<std::string, std::variant<std::string, double, bool, std::nullptr_t> > data_;
    };

    using LuaValue = std::variant<std::string, double, bool, std::nullptr_t, LuaTable>;
    using LuaParameters = std::vector<LuaValue>;
    using NativeFunctionCallback = std::function<LuaParameters(const LuaParameters &)>;
    using EventCallback = std::function<void(const LuaParameters &)>;
    class IBridgeProvider;

    struct EventListener {
        size_t id;
        std::function<void(const LuaParameters &)> callback;
    };

    template<typename T>
    using BridgeResult = std::expected<T, std::string>;

    class EventListenerHandle {
    public:
        EventListenerHandle(std::string event_name, size_t listener_id);

        ~EventListenerHandle();

        EventListenerHandle(const EventListenerHandle &) = delete;

        EventListenerHandle &operator=(const EventListenerHandle &) = delete;

        EventListenerHandle(EventListenerHandle &&) noexcept;

        EventListenerHandle &operator=(EventListenerHandle &&) noexcept;

        void unregister();

        [[nodiscard]] bool is_valid() const noexcept;

    private:
        std::string event_name;
        size_t listener_id;
        bool valid;
    };

    class IBridgeProvider {
    public:
        virtual ~IBridgeProvider() = default;

        virtual BridgeResult<void> register_native_function(
            std::string_view mod_name,
            std::string_view function_name,
            NativeFunctionCallback callback) = 0;

        virtual BridgeResult<LuaParameters> call_luau_function(
            std::string_view mod_name,
            std::string_view function_name,
            const LuaParameters &parameters) const = 0;

        virtual BridgeResult<void> set_shared_data(
            std::string_view key,
            const LuaValue &value) = 0;

        virtual BridgeResult<LuaValue> get_shared_data(
            std::string_view key) const = 0;

        virtual BridgeResult<std::unique_ptr<EventListenerHandle> > listen_event(
            std::string_view event_name,
            EventCallback callback) = 0;

        virtual BridgeResult<void> trigger_event(
            std::string_view event_name,
            const LuaParameters &data) = 0;

        virtual std::vector<std::string> get_registered_mods() const = 0;

        virtual std::vector<std::string> get_registered_functions(std::string_view mod_name) const = 0;
    };

    class BridgeProvider final : public GlobalProvider<BridgeProvider>, public IBridgeProvider {
    public:
        static constexpr std::string_view NAME = "bridge";

        RML_EXPORT BridgeProvider();

        RML_EXPORT ~BridgeProvider() override;

        bool register_globals(lua_State *L) noexcept override;

        RML_EXPORT [[nodiscard]] BridgeResult<void> register_native_function(
            std::string_view mod_name,
            std::string_view function_name,
            NativeFunctionCallback callback) override;

        RML_EXPORT BridgeResult<LuaParameters> call_luau_function(
            std::string_view mod_name,
            std::string_view function_name,
            const LuaParameters &parameters) const override;

        RML_EXPORT [[nodiscard]] BridgeResult<void> set_shared_data(
            std::string_view key,
            const LuaValue &value) override;

        RML_EXPORT BridgeResult<LuaValue> get_shared_data(
            std::string_view key) const override;

        RML_EXPORT BridgeResult<std::unique_ptr<EventListenerHandle> > listen_event(
            std::string_view event_name,
            EventCallback callback) override;

        RML_EXPORT [[nodiscard]] BridgeResult<void> trigger_event(
            std::string_view event_name,
            const LuaParameters &data) override;

        std::vector<std::string> get_registered_mods() const override;

        std::vector<std::string> get_registered_functions(std::string_view mod_name) const override;

        NativeFunctionCallback get_native_function(const std::string &full_name);

        bool has_native_function(const std::string &full_name) const;

        size_t add_event_listener(const std::string &event_name, EventCallback callback);

        void remove_event_listener(const std::string &event_name, size_t listener_id);

        std::vector<EventListener> get_event_listeners(const std::string &event_name);

        static RML_EXPORT BridgeProvider *instance();

        void set_lua_state(lua_State *L) noexcept;

        lua_State *get_lua_state() const noexcept;

        static void register_lua_api(lua_State *L) noexcept;

        friend class EventListenerHandle;

        static LuaValue lua_to_value(lua_State *L, int index);

        static void value_to_lua(lua_State *L, const LuaValue &value);

        static LuaParameters get_lua_parameters(lua_State *L, int start_index);

        static void push_lua_parameters(lua_State *L, const LuaParameters &params);


        mutable std::shared_mutex mutex;
        mutable std::mutex instance_mutex;
        lua_State *lua_state;

        std::unordered_map<std::string, NativeFunctionCallback> native_functions;
        std::unordered_map<std::string, LuaValue> shared_data;
        std::unordered_map<std::string, std::vector<EventListener> > event_listeners;
        std::atomic<size_t> next_listener_id;
    };

    namespace lua_bridge_impl {
        int call_native_function(lua_State *L);

        int register_luau_function(lua_State *L);

        int set_data(lua_State *L);

        int get_data(lua_State *L);

        int listen_event(lua_State *L);

        int trigger_event(lua_State *L);

        int get_mod_list(lua_State *L);

        int get_function_list(lua_State *L);
    }

    static BridgeProvider *g_bridge_provider = nullptr;
}

