#pragma once

#include "RobloxModLoader/internal/engine_abi.hpp"
#include "RobloxModLoader/roblox/util/standard_out.hpp"
#include "lua.h"

namespace RBX::Security
{
	enum class Identity : std::uint64_t;
}

namespace functions
{
	using get_string_atom = uintptr_t (*)(const char* name);
	using descriptor_lookup = uintptr_t* (*)(uintptr_t class_descriptor_hash, uintptr_t* member_descriptor_hash);
	using get_scheduler = uintptr_t (*)();
	using print = void(RML_ENGINE_CALL*)(RBX::MessageType level, const char* fmt, ...);
	using luaH_new = void*(RML_ENGINE_CALL*)(void* L, int32_t narray, int32_t nhash);
	using freeblock = void(RML_ENGINE_CALL*)(lua_State* L, int32_t sizeClass, void* block);
	using lua_pushvalue = void*(RML_ENGINE_CALL*)(lua_State * L, int idx);
	using luaE_newthread = lua_State*(RML_ENGINE_CALL*)(lua_State * L);
	using luau_execute = void(RML_ENGINE_CALL*)(lua_State* L);
	using luau_load = lua_Status(RML_ENGINE_CALL*)(lua_State* L, const char* chunkname, const char* data, size_t size, int env);
	using lua_setfield = void(RML_ENGINE_CALL*)(lua_State* L, int idx, const char* k);
	using luaD_rawrunprotected = int(RML_ENGINE_CALL*)(lua_State* L, void (*PFunc)(lua_State*, void*), void* ud);
	using lua_newthread = lua_State*(RML_ENGINE_CALL*)(lua_State * L);
	using luaD_throw = void(RML_ENGINE_CALL*)(lua_State* L, int errcode);
	using get_global_state = lua_State*(RML_ENGINE_CALL*)(void* script_context, const RBX::Security::Identity* identity, const uint64_t* script);
	using object_create_by_name = uintptr_t (*)(uintptr_t* out, uintptr_t engine_context, uintptr_t name, uint32_t creator_role);
	using instance_bridge_push = void(RML_ENGINE_CALL*)(lua_State* L, uintptr_t instance);
	using task_defer = int(RML_ENGINE_CALL*)(lua_State* L);
	using build_menu_bar_from_dom = void*(RML_ENGINE_CALL*)(void* out_menu_bar, void* dom, void* context);
	using signal_disconnect = void(RML_ENGINE_CALL*)(void* slot);
	using signal_slot_free = void(RML_ENGINE_CALL*)(void* slot);
	using signal_mutex_get = void*(RML_ENGINE_CALL*)();
}
