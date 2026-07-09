#pragma once

#if _WIN32
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#endif

#include <cstdint>

namespace dumper
{
	struct luau
	{
		uintptr_t luaF_newLclosure{};
		uintptr_t luaF_newCclosure{};
		uintptr_t luaF_newproto{};
		uintptr_t luaF_freeproto{};
		uintptr_t luaF_findupval{};
		uintptr_t luaU_load{};
		uintptr_t luaM_free{};
		uintptr_t luaD_reallocstack{};
		uintptr_t luaD_reallocCI{};
		uintptr_t luaH_new{};
		uintptr_t setnodevector{};
		uintptr_t propagatemark{};
		uintptr_t traversetable{};
		uintptr_t lua_resume{};
		uintptr_t index2addr{};
	};
}
