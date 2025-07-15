#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/roblox/script_context.hpp"

#include "pointers.hpp"

namespace RBX {
    lua_State *ScriptContext::get_global_state() {
        if (!g_pointers) return nullptr;

        if (!g_pointers->m_roblox_pointers.get_global_state) {
            LOG_ERROR("[ScriptContext] get_global_state pointer is null, cannot get global state.");
            return nullptr;
        }

        constexpr auto identity = 0ull;
        constexpr auto script = 0ull;

        return g_pointers->m_roblox_pointers.get_global_state(
            this, &identity, &script
        );
    }
}
