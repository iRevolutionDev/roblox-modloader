#include "RobloxModLoader/common.hpp"
#include "lstate.h"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/luau/roblox_extra_space.hpp"

lua_Status *hooks::luau_load(lua_State *L, const char *chunkname, const char *data, size_t size, int env) {
  return hooking::get_original<&hooks::luau_load>()(L, chunkname, data, size, env);
}
