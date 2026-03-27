#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/luau/environment/bridge_provider.hpp"
#include <spdlog/spdlog.h>
#include <format>
#include <algorithm>
#include <ranges>
#include <lualib.h>
#include <lua.h>
#include <cstdlib>
#include <cstring>

namespace rml::luau::environment {
    EventListenerHandle::EventListenerHandle(std::string event_name, const size_t listener_id)
        : event_name(std::move(event_name)), listener_id(listener_id), valid(true) {
    }

    EventListenerHandle::~EventListenerHandle() {
        if (valid) {
            unregister();
        }
    }

    EventListenerHandle::EventListenerHandle(EventListenerHandle &&other) noexcept
        : event_name(std::move(other.event_name))
          , listener_id(other.listener_id)
          , valid(other.valid) {
        other.valid = false;
    }

    EventListenerHandle &EventListenerHandle::operator=(EventListenerHandle &&other) noexcept {
        if (this != &other) {
            if (valid) {
                unregister();
            }
            event_name = std::move(other.event_name);
            listener_id = other.listener_id;
            valid = other.valid;
            other.valid = false;
        }
        return *this;
    }

    void EventListenerHandle::unregister() {
        if (valid) {
            BridgeProvider::instance()->remove_event_listener(event_name, listener_id);
            valid = false;
        }
    }

    bool EventListenerHandle::is_valid() const noexcept {
        return valid;
    }

    BridgeProvider::BridgeProvider() : lua_state(nullptr), next_listener_id(1) {
        g_bridge_provider = this;
    }

    BridgeProvider::~BridgeProvider() {
        std::lock_guard lock(instance_mutex);
        lua_state = nullptr;
        g_bridge_provider = nullptr;
    }

    bool BridgeProvider::register_globals(lua_State *L) noexcept {
        try {
            set_lua_state(L);
            register_lua_api(L);
            return true;
        } catch (const std::exception &e) {
            LOG_ERROR("Bridge: Failed to register globals: {}", e.what());
            return false;
        }
    }

    void BridgeProvider::set_lua_state(lua_State *L) noexcept {
        std::lock_guard lock(instance_mutex);
        lua_state = L;
    }

    lua_State *BridgeProvider::get_lua_state() const noexcept {
        std::lock_guard lock(instance_mutex);
        return lua_state;
    }

    BridgeResult<void> BridgeProvider::register_native_function(
        std::string_view mod_name,
        std::string_view function_name,
        NativeFunctionCallback callback) {
        if (mod_name.empty() || function_name.empty()) {
            return std::unexpected("Mod name and function name cannot be empty");
        }

        const std::string full_name = std::format("{}.{}", mod_name, function_name);

        std::unique_lock lock(mutex);
        native_functions[full_name] = std::move(callback);

        return {};
    }

    BridgeResult<LuaParameters> BridgeProvider::call_luau_function(
        std::string_view mod_name,
        std::string_view function_name,
        const LuaParameters &parameters) const {
        std::lock_guard lock(instance_mutex);
        if (!lua_state) {
            return std::unexpected("Lua state not available");
        }

        try {
            std::string full_name = std::format("{}.{}", mod_name, function_name);
            lua_getglobal(lua_state, mod_name.data());
            if (!lua_istable(lua_state, -1)) {
                lua_pop(lua_state, 1);
                return std::unexpected(std::format("Mod '{}' not found", mod_name));
            }
            lua_getfield(lua_state, -1, function_name.data());
            if (!lua_isfunction(lua_state, -1)) {
                lua_pop(lua_state, 2);
                return std::unexpected(std::format("Function '{}.{}' not found", mod_name, function_name));
            }
            push_lua_parameters(lua_state, parameters);
            if (lua_pcall(lua_state, static_cast<int>(parameters.size()), LUA_MULTRET, 0) != LUA_OK) {
                std::string error = lua_tostring(lua_state, -1);
                lua_pop(lua_state, 2); // error + mod table
                return std::unexpected(std::format("Error calling function '{}': {}", full_name, error));
            }

            const int num_results = lua_gettop(lua_state) - 1;
            LuaParameters results = get_lua_parameters(lua_state, 2);

            lua_pop(lua_state, num_results + 1);

            return results;
        } catch (const std::exception &e) {
            return std::unexpected(std::format("Exception calling Luau function: {}", e.what()));
        }
    }

    BridgeResult<void> BridgeProvider::set_shared_data(std::string_view key, const LuaValue &value) {
        if (key.empty()) {
            return std::unexpected("Key cannot be empty");
        }

        std::unique_lock lock(mutex);
        shared_data[std::string(key)] = value;
        return {};
    }

    BridgeResult<LuaValue> BridgeProvider::get_shared_data(std::string_view key) const {
        if (key.empty()) {
            return std::unexpected("Key cannot be empty");
        }

        std::shared_lock lock(mutex);
        const auto it = shared_data.find(std::string(key));
        return it != shared_data.end() ? it->second : LuaValue{nullptr};
    }

    BridgeResult<std::unique_ptr<EventListenerHandle> > BridgeProvider::listen_event(
        const std::string_view event_name,
        EventCallback callback) {
        if (event_name.empty()) {
            return std::unexpected("Event name cannot be empty");
        }

        size_t id = add_event_listener(std::string(event_name), std::move(callback));
        return std::make_unique<EventListenerHandle>(std::string(event_name), id);
    }

    BridgeResult<void> BridgeProvider::trigger_event(std::string_view event_name, const LuaParameters &data) {
        if (event_name.empty()) {
            return std::unexpected("Event name cannot be empty");
        }

        for (auto listeners = get_event_listeners(std::string(event_name)); const auto &[id, callback]: listeners) {
            try {
                callback(data);
            } catch (const std::exception &e) {
                LOG_ERROR("Bridge: Error in event listener for '{}': {}", event_name, e.what());
            }
        }

        return {};
    }

    std::vector<std::string> BridgeProvider::get_registered_mods() const {
        std::shared_lock lock(mutex);
        std::vector<std::string> mods;

        for (const auto &full_name: native_functions | std::views::keys) {
            if (const auto dot_pos = full_name.find('.'); dot_pos != std::string::npos) {
                if (std::string mod_name = full_name.substr(0, dot_pos);
                    std::ranges::find(mods, mod_name) == mods.end()) {
                    mods.push_back(mod_name);
                }
            }
        }

        return mods;
    }

    std::vector<std::string> BridgeProvider::get_registered_functions(const std::string_view mod_name) const {
        std::shared_lock lock(mutex);
        std::vector<std::string> functions;

        for (const auto &full_name: native_functions | std::views::keys) {
            if (const auto dot_pos = full_name.find('.');
                dot_pos != std::string::npos && full_name.substr(0, dot_pos) == mod_name) {
                functions.push_back(full_name.substr(dot_pos + 1));
            }
        }

        return functions;
    }

    NativeFunctionCallback BridgeProvider::get_native_function(const std::string &full_name) {
        std::shared_lock lock(mutex);
        const auto it = native_functions.find(full_name);
        return it != native_functions.end() ? it->second : NativeFunctionCallback{};
    }

    bool BridgeProvider::has_native_function(const std::string &full_name) const {
        std::shared_lock lock(mutex);
        return native_functions.contains(full_name);
    }

    size_t BridgeProvider::add_event_listener(const std::string &event_name, EventCallback callback) {
        std::unique_lock lock(mutex);
        const size_t id = next_listener_id++;
        event_listeners[event_name].push_back({id, std::move(callback)});
        return id;
    }

    void BridgeProvider::remove_event_listener(const std::string &event_name, size_t listener_id) {
        std::unique_lock lock(mutex);
        if (const auto it = event_listeners.find(event_name); it != event_listeners.end()) {
            auto &listeners = it->second;
            std::erase_if(listeners,
                          [listener_id](const EventListener &l) { return l.id == listener_id; });
        }
    }

    std::vector<EventListener> BridgeProvider::get_event_listeners(const std::string &event_name) {
        std::shared_lock lock(mutex);
        const auto it = event_listeners.find(event_name);
        return it != event_listeners.end() ? it->second : std::vector<EventListener>{};
    }

    BridgeProvider *BridgeProvider::instance() {
        return g_bridge_provider;
    }

    void BridgeProvider::register_lua_api(lua_State *L) noexcept {
        try {
            lua_getglobal(L, "rml");
            if (!lua_istable(L, -1)) {
                lua_pop(L, 1);
                lua_newtable(L);
                lua_setglobal(L, "rml");
                lua_getglobal(L, "rml");
            }

            lua_newtable(L);
            lua_pushcfunction(L, lua_bridge_impl::call_native_function, "call_native");
            lua_setfield(L, -2, "call");
            lua_pushcfunction(L, lua_bridge_impl::register_luau_function, "register_luau");
            lua_setfield(L, -2, "register");
            lua_pushcfunction(L, lua_bridge_impl::set_data, "set_data");
            lua_setfield(L, -2, "set_data");
            lua_pushcfunction(L, lua_bridge_impl::get_data, "get_data");
            lua_setfield(L, -2, "get_data");
            lua_pushcfunction(L, lua_bridge_impl::listen_event, "listen_event");
            lua_setfield(L, -2, "listen_event");
            lua_pushcfunction(L, lua_bridge_impl::trigger_event, "trigger_event");
            lua_setfield(L, -2, "trigger_event");

            lua_pushcfunction(L, lua_bridge_impl::get_mod_list, "get_mod_list");
            lua_setfield(L, -2, "get_mod_list");

            lua_pushcfunction(L, lua_bridge_impl::get_function_list, "get_function_list");
            lua_setfield(L, -2, "get_function_list");

            lua_setfield(L, -2, "bridge");

            lua_pop(L, 1);
        } catch (const std::exception &e) {
            LOG_ERROR("Bridge: Failed to register Lua API: {}", e.what());
        }
    }

    LuaValue BridgeProvider::lua_to_value(lua_State *L, const int index) {
        switch (lua_type(L, index)) {
            case LUA_TSTRING:
                return std::string(lua_tostring(L, index));
            case LUA_TNUMBER:
                return lua_tonumber(L, index);
            case LUA_TBOOLEAN:
                return static_cast<bool>(lua_toboolean(L, index));
            case LUA_TTABLE: {
                LuaTable table;
                lua_pushnil(L); // First key
                while (lua_next(L, index < 0 ? index - 1 : index) != 0) {
                    // Stack: ..., key, value
                    if (lua_type(L, -2) == LUA_TSTRING) {
                        std::string key = lua_tostring(L, -2);

                        switch (lua_type(L, -1)) {
                            case LUA_TSTRING:
                                table.set(key, std::string(lua_tostring(L, -1)));
                                break;
                            case LUA_TNUMBER:
                                table.set(key, lua_tonumber(L, -1));
                                break;
                            case LUA_TBOOLEAN:
                                table.set(key, static_cast<bool>(lua_toboolean(L, -1)));
                                break;
                            case LUA_TNIL:
                                table.set(key, nullptr);
                                break;
                            default:
                                break;
                        }
                    }
                    lua_pop(L, 1); // Remove value, keep key for next iteration
                }
                return table;
            }
            case LUA_TNIL:
            default:
                return nullptr;
        }
    }

    void BridgeProvider::value_to_lua(lua_State *L, const LuaValue &value) {
        std::visit([L]<typename Type>(const Type &v) {
            using T = std::decay_t<Type>;
            if constexpr (std::is_same_v<T, std::string>) {
                lua_pushstring(L, v.c_str());
            } else if constexpr (std::is_same_v<T, double>) {
                lua_pushnumber(L, v);
            } else if constexpr (std::is_same_v<T, bool>) {
                lua_pushboolean(L, v);
            } else if constexpr (std::is_same_v<T, LuaTable>) {
                lua_newtable(L);
                for (const auto &[key, table_value]: v) {
                    lua_pushstring(L, key.c_str());
                    std::visit([L]<typename TableType>(const TableType &tv) {
                        using TT = std::decay_t<TableType>;
                        if constexpr (std::is_same_v<TT, std::string>) {
                            lua_pushstring(L, tv.c_str());
                        } else if constexpr (std::is_same_v<TT, double>) {
                            lua_pushnumber(L, tv);
                        } else if constexpr (std::is_same_v<TT, bool>) {
                            lua_pushboolean(L, tv);
                        } else {
                            lua_pushnil(L);
                        }
                    }, table_value);
                    lua_settable(L, -3);
                }
            } else {
                lua_pushnil(L);
            }
        }, value);
    }

    LuaParameters BridgeProvider::get_lua_parameters(lua_State *L, int start_index) {
        LuaParameters params;
        const int top = lua_gettop(L);

        for (int i = start_index; i <= top; ++i) {
            params.push_back(lua_to_value(L, i));
        }

        return params;
    }

    void BridgeProvider::push_lua_parameters(lua_State *L, const LuaParameters &params) {
        for (const auto &param: params) {
            value_to_lua(L, param);
        }
    }

    namespace lua_bridge_impl {
        int call_native_function(lua_State *L) {
            try {
                if (lua_gettop(L) < 2) {
                    luaL_error(L, "callNative requires at least 2 arguments: mod_name, function_name");
                }

                std::string mod_name = luaL_checkstring(L, 1);
                std::string function_name = luaL_checkstring(L, 2);
                const std::string full_name = std::format("{}.{}", mod_name, function_name);

                const auto callback = BridgeProvider::instance()->get_native_function(full_name);
                if (!callback) {
                    luaL_error(L, "Native function '%s' not found", full_name.c_str());
                }

                const LuaParameters params = BridgeProvider::get_lua_parameters(L, 3);

                const LuaParameters results = callback(params);
                BridgeProvider::push_lua_parameters(L, results);

                return static_cast<int>(results.size());
            } catch (const std::exception &e) {
                luaL_error(L, "Error calling native function: %s", e.what());
            }
        }

        int register_luau_function(lua_State *L) {
            try {
                if (lua_gettop(L) < 3) {
                    luaL_error(L, "registerLuau requires 3 arguments: mod_name, function_name, function");
                }

                const std::string mod_name = luaL_checkstring(L, 1);
                const std::string function_name = luaL_checkstring(L, 2);

                if (!lua_isfunction(L, 3)) {
                    luaL_error(L, "Third argument must be a function");
                }

                lua_getglobal(L, mod_name.c_str());
                if (!lua_istable(L, -1)) {
                    lua_pop(L, 1);
                    lua_newtable(L);
                    lua_setglobal(L, mod_name.c_str());
                    lua_getglobal(L, mod_name.c_str());
                }

                lua_pushvalue(L, 3);
                lua_setfield(L, -2, function_name.c_str());
                lua_pop(L, 1);

                return 0;
            } catch (const std::exception &e) {
                luaL_error(L, "Error registering Luau function: %s", e.what());
            }
        }

        int set_data(lua_State *L) {
            try {
                if (lua_gettop(L) < 2) {
                    luaL_error(L, "setData requires 2 arguments: key, value");
                }

                const auto bridge = BridgeProvider::instance();
                if (!bridge) {
                    luaL_error(L, "BridgeProvider instance not available");
                }

                const std::string key = luaL_checkstring(L, 1);
                const LuaValue value = BridgeProvider::lua_to_value(L, 2);

                std::unique_lock lock(bridge->mutex);
                bridge->shared_data[key] = value;
                return 0;
            } catch (const std::exception &e) {
                luaL_error(L, "Error setting data: %s", e.what());
            }
        }

        int get_data(lua_State *L) {
            try {
                if (lua_gettop(L) < 1) {
                    luaL_error(L, "getData requires 1 argument: key");
                }

                const std::string key = luaL_checkstring(L, 1);

                std::shared_lock lock(BridgeProvider::instance()->mutex);
                const auto it = BridgeProvider::instance()->shared_data.find(key);
                const LuaValue value = it != BridgeProvider::instance()->shared_data.end()
                                           ? it->second
                                           : LuaValue{nullptr};

                BridgeProvider::value_to_lua(L, value);
                return 1;
            } catch (const std::exception &e) {
                luaL_error(L, "Error getting data: %s", e.what());
            }
        }

        int listen_event(lua_State *L) {
            try {
                if (lua_gettop(L) < 2) {
                    luaL_error(L, "listenEvent requires 2 arguments: event_name, callback");
                }

                const std::string event_name = luaL_checkstring(L, 1);

                if (!lua_isfunction(L, 2)) {
                    luaL_error(L, "Second argument must be a function");
                }

                int callback_ref = lua_ref(L, 1);

                const EventCallback callback = [L, callback_ref](const LuaParameters &params) {
                    lua_rawgeti(L, LUA_REGISTRYINDEX, callback_ref);
                    BridgeProvider::push_lua_parameters(L, params);

                    if (lua_pcall(L, static_cast<int>(params.size()), 0, 0) != LUA_OK) {
                        LOG_ERROR("Bridge: Error in event callback: {}", lua_tostring(L, -1));
                        lua_pop(L, 1);
                    }
                };

                const size_t listener_id = BridgeProvider::instance()->add_event_listener(event_name, callback);
                lua_pushinteger(L, static_cast<lua_Integer>(listener_id));

                return 1;
            } catch (const std::exception &e) {
                luaL_error(L, "Error listening to event: %s", e.what());
            }
        }

        int trigger_event(lua_State *L) {
            try {
                if (lua_gettop(L) < 1) {
                    luaL_error(L, "triggerEvent requires at least 1 argument: event_name");
                }

                const std::string event_name = luaL_checkstring(L, 1);
                const LuaParameters params = BridgeProvider::get_lua_parameters(L, 2);

                for (const auto listeners = BridgeProvider::instance()->get_event_listeners(event_name); const auto &[id
                         , callback]: listeners) {
                    try {
                        callback(params);
                    } catch (const std::exception &e) {
                        LOG_ERROR("Bridge: Error in event listener: {}", e.what());
                    }
                }

                return 0;
            } catch (const std::exception &e) {
                luaL_error(L, "Error triggering event: %s", e.what());
            }
        }

        int get_mod_list(lua_State *L) {
            try {
                const auto mods = BridgeProvider::instance()->get_registered_mods();

                lua_newtable(L);
                for (size_t i = 0; i < mods.size(); ++i) {
                    lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
                    lua_pushstring(L, mods[i].c_str());
                    lua_settable(L, -3);
                }

                return 1;
            } catch (const std::exception &e) {
                luaL_error(L, "Error getting mod list: %s", e.what());
            }
        }

        int get_function_list(lua_State *L) {
            try {
                if (lua_gettop(L) < 1) {
                    luaL_error(L, "getFunctionList requires 1 argument: mod_name");
                }

                const std::string mod_name = luaL_checkstring(L, 1);
                const auto functions = BridgeProvider::instance()->get_registered_functions(mod_name);

                lua_newtable(L);
                for (size_t i = 0; i < functions.size(); ++i) {
                    lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
                    lua_pushstring(L, functions[i].c_str());
                    lua_settable(L, -3);
                }

                return 1;
            } catch (const std::exception &e) {
                luaL_error(L, "Error getting function list: %s", e.what());
            }
        }
    }
}
