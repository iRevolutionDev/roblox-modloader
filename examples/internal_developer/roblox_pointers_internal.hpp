#pragma once

#include "RobloxModLoader/common.hpp"

template<typename T>
class HashTable;

#pragma pack(push, 1)
struct roblox_pointers_internal {
    PVOID m_is_internal;
    uintptr_t m_is_internal_flag;
};
#pragma pack(pop)

static_assert(sizeof(roblox_pointers_internal) % 8 == 0, "Pointers are not properly aligned");
