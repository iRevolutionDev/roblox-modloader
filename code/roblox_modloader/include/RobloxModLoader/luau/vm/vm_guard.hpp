#pragma once

#include "RobloxModLoader/luau/vm/vm_error.hpp"

namespace rml::luau::vm
{
	using GuardedBody = std::expected<int, VmError> (*)(lua_State*);

	[[noreturn]] void raise(lua_State* L, const VmError& error);

	template <GuardedBody Body>
	int guarded(lua_State* L)
	{
		auto result = Body(L);
		if (result)
		{
			return *result;
		}

		raise(L, result.error());
	}
}
