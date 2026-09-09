#include "RobloxModLoader/luau/vm/lua_thread.hpp"

#include "RobloxModLoader/luau/generated/layout_access.hpp"
#include "RobloxModLoader/luau/vm/stack_guard.hpp"
#include "RobloxModLoader/luau/vm/vm_api.hpp"

namespace rml::luau::vm
{
	std::expected<Thread, VmError> Thread::spawn(lua_State* parent, lua_State* anchor)
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

		auto held = Ref::take(parent, -1, anchor);
		lua_pop(parent, 1);

		if (!held.valid())
		{
			return std::unexpected(VmError::internal("could not anchor the new Luau thread in the registry"));
		}

		return Thread{thread, std::move(held)};
	}

	std::expected<ResumeOutcome, VmError> resume(lua_State* L, const int nargs) noexcept
	{
		if (!L)
		{
			return std::unexpected(VmError::internal("resume without a Luau state"));
		}

		if (!api_ready())
		{
			report_api_unavailable_once();
			return std::unexpected(VmError::unavailable("the Luau C API is unavailable on this Studio build"));
		}

		const int status = lua_resume(L, nullptr, nargs);

		if (status == LUA_YIELD || status == LUA_BREAK)
		{
			return ResumeOutcome{.suspended = true};
		}

		if (status != LUA_OK)
		{
			return std::unexpected(error_from_stack(L, kind_from_status(status)));
		}

		return ResumeOutcome{.results = lua_gettop(L)};
	}

	bool is_suspended(const Thread& thread) noexcept
	{
		auto* L = thread.get();
		if (!thread.valid() || !L)
		{
			return false;
		}

		const auto* state = access::state(L);
		return state->isactive || state->status == LUA_YIELD || state->status == LUA_BREAK;
	}

	bool close_thread(lua_State* host, const Thread& thread) noexcept
	{
		if (!host || !thread.valid() || !api_ready())
		{
			return false;
		}

		StackGuard guard(host);

		lua_getglobal(host, "coroutine");
		if (!lua_istable(host, -1))
		{
			return false;
		}

		lua_getfield(host, -1, "close");
		if (!lua_isfunction(host, -1) || !thread.push(host))
		{
			return false;
		}

		return lua_pcall(host, 1, 0, 0) == LUA_OK;
	}
}
