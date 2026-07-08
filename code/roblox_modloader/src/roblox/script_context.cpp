#include "RobloxModLoader/luau/script_context.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/roblox/script_context.hpp"

#include "pointers.hpp"

RML_LOG_SCOPE("ScriptContext");

namespace RBX {
    lua_State *ScriptContext::get_global_state(const Security::Identity identity) {
        if (!g_pointers) return nullptr;

        if (!g_pointers->m_roblox_pointers.get_global_state) {
            RML_ERROR("get_global_state pointer is null, cannot get global state.");
            return nullptr;
        }

        constexpr auto script = 0ull;

        return g_pointers->m_roblox_pointers.get_global_state(
            this, &identity, &script
        );
    }
}
