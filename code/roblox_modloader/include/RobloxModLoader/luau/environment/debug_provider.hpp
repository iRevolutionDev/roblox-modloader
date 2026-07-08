#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include "globals_registry.hpp"

namespace rml::luau::environment
{
	class DebugProvider final : public GlobalProvider<DebugProvider>
	{
	public:
		static constexpr std::string_view NAME = "debug";

		bool register_globals(lua_State* L) noexcept override;

		static void register_debug_table(lua_State* L) noexcept;
	};

	namespace debug_impl
	{
		int getconstants(lua_State* L);

		int getconstant(lua_State* L);

		int setconstant(lua_State* L);

		int getinfo(lua_State* L);

		int getproto(lua_State* L);

		int getprotos(lua_State* L);

		int setstack(lua_State* L);

		int getstack(lua_State* L);

		int debug_setupvalue(lua_State* L);

		int getupvalue(lua_State* L);

		int getupvalues(lua_State* L);
	}
}
