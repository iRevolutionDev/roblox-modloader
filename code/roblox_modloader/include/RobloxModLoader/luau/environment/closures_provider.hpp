#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include "globals_registry.hpp"

namespace rml::luau::environment
{
	class ClosuresProvider final : public GlobalProvider<ClosuresProvider>
	{
	public:
		static constexpr std::string_view NAME = "closures";

		bool register_globals(lua_State* L) noexcept override;
	};

	namespace closures_impl
	{
		int getgc(lua_State* L);

		int iscclosure(lua_State* L);

		int islclosure(lua_State* L);

		int clonefunction(lua_State* L);

		int newlclosure(lua_State* L);

		int hookfunction(lua_State* L);

		int restorefunction(lua_State* L);
	}
}
