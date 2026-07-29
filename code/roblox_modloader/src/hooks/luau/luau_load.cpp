#include "RobloxModLoader/internal/common.hpp"
#include "lstate.h"
#include "RobloxModLoader/internal/function_types.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/roblox/luau/roblox_extra_space.hpp"

lua_Status rml::Hooks::luau_load(lua_State *L, const char *chunkname, const char *data, size_t size, int env) {
  return rml::Hooking::get_original<&rml::Hooks::luau_load>()(L, chunkname, data, size, env);
}
