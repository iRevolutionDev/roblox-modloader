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

  // Scene Manager Render View
  PVOID m_render_view;
};
#pragma pack(pop)
static_assert(sizeof(roblox_pointers) % 8 == 0, "Pointers are not properly aligned");
