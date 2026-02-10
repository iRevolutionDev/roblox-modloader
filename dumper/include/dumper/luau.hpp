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
		uintptr_t luaF_freeproto{};
		uintptr_t table{};
		uintptr_t thread{};
		uintptr_t gc{};
		uintptr_t page{};
		uintptr_t propagatemark{};
	};
}
