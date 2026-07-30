#include "RobloxModLoader/luau/vm/lua_thread.hpp"

#include "RobloxModLoader/luau/vm/vm_api.hpp"

namespace rml::luau::vm
{
	std::expected<Thread, VmError> Thread::spawn(lua_State* parent)
	{
		if (!parent)
		{
			return std::unexpected(VmError::unavailable("cannot spawn a Luau thread without a parent state"));
		}

		if (!api_ready())
		{
			report_api_unavailable_once();
			return std::unexpected(VmError::unavailable("the Luau C API is unavailable on this Studio build"));
		}

		auto* thread = lua_newthread(parent);
		if (!thread)
		{
			return std::unexpected(VmError::unavailable("lua_newthread returned no thread"));
		}

		auto anchor = Ref::take(parent, -1);
		lua_pop(parent, 1);

		if (!anchor.valid())
		{
			return std::unexpected(VmError::internal("could not anchor the new Luau thread in the registry"));
		}

		return Thread{thread, std::move(anchor)};
	}
}
