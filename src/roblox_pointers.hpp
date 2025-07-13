#pragma once
#include "RobloxModLoader/common.hpp"
#include "function_types.hpp"

#include <RobloxModLoader/memory/handle.hpp>

template<typename T>
class HashTable;

// needed for serialization of the pointers cache
#pragma pack(push, 1)
struct roblox_pointers {
  PVOID m_rbx_crash;
  PVOID m_render_prepare;
  PVOID m_render_perform;
  PVOID m_render_pass_2d;
  PVOID m_render_pass_3d;

  PVOID m_profile_log;

  // Scene Manager Render View
  PVOID m_render_view;

  functions::get_scheduler get_scheduler;

  // Script Context
  PVOID resume_waiting_scripts;

  // Lua Functions
  functions::luau_execute luau_execute;
  functions::luau_load luau_load;
  functions::luaE_newthread luaE_newthread;
  functions::lua_pushvalue lua_pushvalue;
  functions::luaH_new luaH_new;
  functions::freeblock freeblock;
  functions::lua_newthread lua_newthread;

  functions::task_defer task_defer;
  functions::get_global_state get_global_state;
};
#pragma pack(pop)
static_assert(sizeof(roblox_pointers) % 8 == 0, "Pointers are not properly aligned");
