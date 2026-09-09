#include "RobloxModLoader/luau/env/binding.hpp"

#include "RobloxModLoader/luau/script_host.hpp"

RML_LOG_SCOPE("LuauMod");

namespace rml::luau
{
	static std::string formatted_message(lua_State* L)
	{
		const auto argc = lua_gettop(L);

		std::size_t length = 0;
		const auto* first = luaL_checklstring(L, 1, &length);

		if (argc <= 1)
		{
			return std::string{first, length};
		}

		const auto base = lua_gettop(L);

		lua_getglobal(L, "string");
		lua_getfield(L, -1, "format");
		for (auto i = 1; i <= argc; ++i)
		{
			lua_pushvalue(L, i);
		}
		lua_call(L, argc, 1);

		std::size_t result_length = 0;
		const auto* text = lua_tolstring(L, -1, &result_length);
		std::string message{text ? text : "", text ? result_length : 0};

		lua_settop(L, base);
		return message;
	}

	static int log_info(lua_State* L)
	{
		LOG_INFO("[{}] {}", bound_env(L).mod().mod_name(), formatted_message(L));
		return 0;
	}

	static int log_warn(lua_State* L)
	{
		LOG_WARN("[{}] {}", bound_env(L).mod().mod_name(), formatted_message(L));
		return 0;
	}

	static int log_error(lua_State* L)
	{
		LOG_ERROR("[{}] {}", bound_env(L).mod().mod_name(), formatted_message(L));
		return 0;
	}

	static int log_debug(lua_State* L)
	{
		LOG_DEBUG("[{}] {}", bound_env(L).mod().mod_name(), formatted_message(L));
		return 0;
	}

	static void push_log_table(ScriptEnv& env, lua_State* L)
	{
		lua_newtable(L);

		push_bound_function(env, L, "info", &log_info);
		lua_setfield(L, -2, "info");

		push_bound_function(env, L, "warn", &log_warn);
		lua_setfield(L, -2, "warn");

		push_bound_function(env, L, "error", &log_error);
		lua_setfield(L, -2, "error");

		push_bound_function(env, L, "debug", &log_debug);
		lua_setfield(L, -2, "debug");

		lua_setreadonly(L, -1, true);
	}

	static int rml_on_unload(lua_State* L)
	{
		luaL_checktype(L, 1, LUA_TFUNCTION);

		auto& env = bound_env(L);
		env.add_unload_handler(anchor_in_env(env, L, 1));

		return 0;
	}

	static void push_mod_table(const ScriptEnv& env, lua_State* L)
	{
		const auto& manifest = env.mod().manifest;

		lua_newtable(L);

		lua_pushstring(L, manifest ? manifest->name.c_str() : "");
		lua_setfield(L, -2, "name");

		lua_pushstring(L, manifest ? manifest->version.c_str() : "");
		lua_setfield(L, -2, "version");

		lua_pushstring(L, manifest ? manifest->author.c_str() : "");
		lua_setfield(L, -2, "author");

		lua_pushstring(L, manifest ? manifest->description.c_str() : "");
		lua_setfield(L, -2, "description");

		const auto path = manifest ? manifest->root.generic_string() : std::string{};
		lua_pushstring(L, path.c_str());
		lua_setfield(L, -2, "path");

		lua_setreadonly(L, -1, true);
	}

	bool bind_rml(ScriptEnv& env, lua_State* L) noexcept
	{
		lua_newtable(L);

		push_log_table(env, L);
		lua_setfield(L, -2, "log");

		push_mod_table(env, L);
		lua_setfield(L, -2, "mod");

		push_bound_function(env, L, "on_unload", &rml_on_unload);
		lua_setfield(L, -2, "on_unload");

		lua_setreadonly(L, -1, true);
		lua_setglobal(L, "rml");

		return true;
	}
}
