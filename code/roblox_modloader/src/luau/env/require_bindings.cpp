#include "RobloxModLoader/luau/env/binding.hpp"

#include "RobloxModLoader/luau/modules/module_registry.hpp"
#include "RobloxModLoader/luau/modules/module_resolver.hpp"
#include "RobloxModLoader/luau/script_host.hpp"

RML_LOG_SCOPE("Require");

namespace rml::luau
{
	static int forward_to_engine(lua_State* L, const ScriptEnv& env)
	{
		if (!env.original_require().valid())
		{
			luaL_error(L, "require: this thread has no engine require to fall back to");
		}

		const auto argc = lua_gettop(L);

		env.original_require().push(L);
		lua_insert(L, 1);
		lua_call(L, argc, LUA_MULTRET);

		return lua_gettop(L);
	}

	static int require_binding(lua_State* L)
	{
		auto& env = bound_env(L);

		if (lua_gettop(L) >= 1 && lua_isuserdata(L, 1))
		{
			return forward_to_engine(L, env);
		}

		std::size_t length = 0;
		const auto* specifier = luaL_checklstring(L, 1, &length);

		const auto resolved = resolve_module(std::string_view{specifier, length}, env.mod());
		if (!resolved)
		{
			const auto message = resolved.error().describe();
			luaL_error(L, "%s", message.c_str());
		}

		if (const auto loaded = env.modules().require(L, *resolved, env.mod()); !loaded)
		{
			const auto message = loaded.error().message;
			luaL_error(L, "%s", message.c_str());
		}

		return 1;
	}

	bool bind_require(ScriptEnv& env, lua_State* L) noexcept
	{
		lua_getglobal(L, "require");
		if (lua_isfunction(L, -1))
		{
			env.adopt_original_require(vm::Ref::take(L, -1));
		}
		else
		{
			RML_DEBUG("No engine require on the thread for mod '{}'", env.mod().mod_name());
		}
		lua_pop(L, 1);

		set_bound_global(env, L, "require", &require_binding);
		return true;
	}
}
