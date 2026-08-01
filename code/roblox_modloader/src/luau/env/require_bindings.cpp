#include "RobloxModLoader/luau/env/binding.hpp"
#include "RobloxModLoader/luau/generated/layout_access.hpp"
#include "RobloxModLoader/luau/modules/module_resolver.hpp"
#include "RobloxModLoader/luau/script_host.hpp"

RML_LOG_SCOPE("Require");

namespace rml::luau
{
	static constexpr int kRequirerSearchDepth = 24;

	static std::string requirer_of(lua_State* L)
	{
		alignas(16) std::array<std::byte, 1024> storage{};
		const auto* record = access::debug_record(storage.data());

		for (auto level = 1; level <= kRequirerSearchDepth; ++level)
		{
			storage.fill(std::byte{});

			if (!lua_getinfo(L, level, "s", reinterpret_cast<lua_Debug*>(storage.data())))
			{
				break;
			}

			if (record->source == nullptr)
			{
				continue;
			}

			std::string_view name{record->source};
			if (name.starts_with('='))
			{
				name.remove_prefix(1);
			}

			if (is_logical_module_path(name))
			{
				return std::string{name};
			}
		}

		return {};
	}

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

	static int require_node(lua_State* L, ScriptEnv& env, const ScriptNode& node)
	{
		if (!node.requireable())
		{
			luaL_error(L, "attempted to require %s \"%s\", only a ModuleScript can be required",
			           std::string{node.class_name()}.c_str(), node.full_name().c_str());
		}

		const ResolvedModule resolved{.id = ModuleId{node.source.generic_string()}, .logical = node.logical};

		if (const auto loaded = env.modules().require(L, resolved, env); !loaded)
		{
			const auto message = loaded.error().message;
			luaL_error(L, "%s", message.c_str());
		}

		return 1;
	}

	static int require_binding(lua_State* L)
	{
		auto& env = bound_env(L);

		if (lua_gettop(L) < 1)
		{
			luaL_error(L, "require expects a ModuleScript, an asset id, or a path such as '@self/name'");
		}

		if (lua_type(L, 1) != LUA_TSTRING)
		{
			if (const auto* node = to_script_node(L, 1, &env))
			{
				return require_node(L, env, *node);
			}

			return forward_to_engine(L, env);
		}

		std::size_t length = 0;
		const auto* specifier = lua_tolstring(L, 1, &length);
		const std::string_view text{specifier, length};

		const auto resolved = resolve_module(text, env.mod(), text.starts_with('.') ? requirer_of(L) : std::string{});
		if (!resolved)
		{
			const auto message = resolved.error().describe();
			luaL_error(L, "%s", message.c_str());
		}

		if (const auto loaded = env.modules().require(L, *resolved, env); !loaded)
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
