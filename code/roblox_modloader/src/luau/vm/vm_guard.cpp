#include "RobloxModLoader/luau/vm/vm_guard.hpp"

#if defined(_MSC_VER)
	#define RML_UNREACHABLE() __assume(0)
#else
	#define RML_UNREACHABLE() __builtin_unreachable()
#endif

namespace rml::luau::vm
{
	void raise(lua_State* L, const VmError& error)
	{
		{
			const std::string text = error.describe();
			lua_pushlstring(L, text.data(), text.size());
		}

		lua_error(L);

		RML_UNREACHABLE();
	}
}
