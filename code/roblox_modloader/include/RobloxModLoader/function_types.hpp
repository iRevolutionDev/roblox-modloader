#pragma once

#include "RobloxModLoader/roblox/util/standard_out.hpp"
#include "lua.h"

#include <string>

namespace RBX::Security
{
	enum class Identity : std::uint64_t;
}

namespace functions
{
	using get_string_atom = uintptr_t (*)(const char* name);
	using descriptor_lookup = uintptr_t* (*)(uintptr_t class_descriptor_hash, uintptr_t* member_descriptor_hash);
	using get_scheduler = uintptr_t (*)();
	using print = void(__fastcall*)(RBX::MessageType level, const char* fmt, ...);
	using luaH_new = void*(__fastcall*)(void* L, int32_t narray, int32_t nhash);
	using freeblock = void(__fastcall*)(lua_State* L, int32_t sizeClass, void* block);
	using lua_pushvalue = void*(__fastcall*)(lua_State * L, int idx);
	using luaE_newthread = lua_State*(__fastcall*)(lua_State * L);
	using luau_execute = void(__fastcall*)(lua_State* L);
	using luau_load = lua_Status(__fastcall*)(lua_State* L, const char* chunkname, const char* data, size_t size, int env);
	using lua_setfield = void(__fastcall*)(lua_State* L, int idx, const char* k);
	using luaD_rawrunprotected = int(__fastcall*)(lua_State* L, void (*PFunc)(lua_State*, void*), void* ud);
	using lua_newthread = lua_State*(__fastcall*)(lua_State * L);
	using luaD_throw = void(__fastcall*)(lua_State* L, int errcode);
	using get_global_state = lua_State*(__fastcall*)(void* script_context, const RBX::Security::Identity* identity, const uint64_t* script);
	using creator_create_by_name = uintptr_t (*)(uintptr_t* out, const std::string& name, uint32_t creator_role);
	using instance_bridge_push = void(__fastcall*)(lua_State* L, uintptr_t instance);
	using task_defer = int(__fastcall*)(lua_State* L);
}
