#include "RobloxModLoader/luau/vm/thread_identity.hpp"

#include "RobloxModLoader/luau/generated/layout_access.hpp"
#include "RobloxModLoader/roblox/luau/roblox_extra_space.hpp"
#include "utils/seh_guard.hpp"

#include "lstate.h"
#include "lobject.h"

RML_LOG_SCOPE("ThreadIdentity");

namespace rml::luau::vm
{
	struct IdentityWrite
	{
		const lua_State* L;
		RBX::Security::Permissions identity;
		std::uint64_t capabilities;
	};

	struct IdentityRead
	{
		const lua_State* L;
		RBX::Security::Permissions identity;
		std::uint64_t capabilities;
		bool found;
	};

	static void write_identity(void* ctx)
	{
		const auto* call = static_cast<IdentityWrite*>(ctx);
		auto* extra_space = static_cast<RBX::Luau::RobloxExtraSpace*>(call->L->userdata);
		if (!extra_space) return;
		extra_space->context.identity = call->identity;
		extra_space->capabilities = call->capabilities;
	}

	static void read_identity(void* ctx)
	{
		auto* call = static_cast<IdentityRead*>(ctx);
		const auto* extra_space = static_cast<const RBX::Luau::RobloxExtraSpace*>(call->L->userdata);
		if (!extra_space) return;
		call->identity = extra_space->context.identity;
		call->capabilities = extra_space->capabilities;
		call->found = true;
	}

	static void report_elevation_unavailable()
	{
		static std::once_flag reported;
		std::call_once(reported, [] {
			RML_ERROR("Prototype elevation is off: the dumper has not recovered Proto.userdata for this Studio "
			          "build, and the slot luau declares for it holds something else here");
		});
	}

	bool set_identity(lua_State* L, const RBX::Security::Permissions identity, const std::uint64_t capabilities) noexcept
	{
		if (!L)
		{
			RML_ERROR("Cannot set thread identity: Lua state is null");
			return false;
		}

		IdentityWrite call{L, identity, capabilities};
		if (!utils::guarded_invoke(&write_identity, &call))
		{
			RML_ERROR("set_thread_identity faulted - lua_State/extra-space layout may have changed on this "
			          "Studio build; skipping identity set");
			return false;
		}

		RML_INFO("Set thread identity to {} with capabilities 0x{:X}", static_cast<int>(identity), capabilities);
		return true;
	}

	IdentityScope::IdentityScope(lua_State* L, const RBX::Security::Permissions identity, const std::uint64_t capabilities) noexcept
		: m_state(L)
	{
		if (!L)
		{
			return;
		}

		IdentityRead current{L, {}, 0, false};
		if (!utils::guarded_invoke(&read_identity, &current) || !current.found)
		{
			return;
		}

		m_previous_identity = current.identity;
		m_previous_capabilities = current.capabilities;
		m_restore = set_identity(L, identity, capabilities);
	}

	IdentityScope::~IdentityScope()
	{
		if (!m_restore)
		{
			return;
		}

		set_identity(m_state, m_previous_identity, m_previous_capabilities);
	}

	void elevate_closure(const Closure* closure, std::uint64_t) noexcept
	{
		if (!closure || access::closure(closure)->isC)
		{
			return;
		}

		report_elevation_unavailable();
	}
}
