#pragma once

#include "lua.h"

namespace functions {
    using get_scheduler = uintptr_t(*)();
    using print = void(__fastcall *)(int level, const char *fmt, ...);
    using luaH_new = void *(__fastcall *)(void *L, int32_t narray, int32_t nhash);
    using freeblock = void(__fastcall *)(lua_State *L, int32_t sizeClass, void *block);
    using lua_pushvalue = void(__fastcall *)(lua_State *L, int idx);
    using luaE_newthread = lua_State *(__fastcall *)(lua_State *L);
    using luau_load = lua_Status(__fastcall*)(lua_State *L, const char *chunkname, const char *data, size_t size,
                                              int env);
    using lua_newthread = lua_State *(__fastcall *)(lua_State *L);
    using get_global_state = lua_State *(__fastcall *)(void *scriptContext, const uint64_t *identity,
                                                       const uint64_t *unk_0);
}
