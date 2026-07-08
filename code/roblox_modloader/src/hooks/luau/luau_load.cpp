#include "RobloxModLoader/internal/common.hpp"
#include "lstate.h"
#include "RobloxModLoader/internal/function_types.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/roblox/luau/roblox_extra_space.hpp"

static_assert(std::is_same_v<decltype(&hooks::luau_load), functions::luau_load>);

lua_Status hooks::luau_load(lua_State *L, const char *chunkname, const char *data, size_t size, int env) {
  return hooking::get_original<&hooks::luau_load>()(L, chunkname, data, size, env);
}
