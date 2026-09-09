#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/roblox/script_context.hpp"
#include "pointers.hpp"
#include "utils/seh_guard.hpp"

RML_LOG_SCOPE("ScriptContext");

namespace RBX
{
	struct GetGlobalStateCall
	{
		ScriptContext* self;
		const Security::Identity* identity;
		const std::uint64_t* script;
		lua_State* result;
	};

	static void invoke_get_global_state(void* ctx)
	{
		auto* call = static_cast<GetGlobalStateCall*>(ctx);
		call->result = g_pointers->m_roblox_pointers.get_global_state(call->self, call->identity, call->script);
	}

	lua_State* ScriptContext::get_global_state(const Security::Identity identity)
	{
		if (!g_pointers)
			return nullptr;

		if (!g_pointers->m_roblox_pointers.get_global_state)
		{
			RML_ERROR("get_global_state pointer is null, cannot get global state.");
			return nullptr;
		}

		constexpr auto script = 0ull;

		GetGlobalStateCall call{this, &identity, &script, nullptr};
		if (!rml::utils::guarded_invoke(&invoke_get_global_state, &call))
		{
			RML_ERROR("getGlobalState faulted - the ScriptContext offset likely changed on this Studio "
			          "build; Luau is skipped (Studio stays alive)");
			return nullptr;
		}

		return call.result;
	}
}
