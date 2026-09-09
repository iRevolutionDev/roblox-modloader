#include "seh_guard.hpp"

#include "RobloxModLoader/internal/platform.hpp"

#if defined(RML_WINDOWS)
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
#endif

namespace rml::utils
{
#if defined(RML_WINDOWS)
	static int access_violation_filter(const unsigned int code) noexcept
	{
		return code == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
	}

	bool guarded_invoke(void (*fn)(void* ctx), void* ctx) noexcept
	{
		__try
		{
			fn(ctx);
			return true;
		}
		__except (access_violation_filter(GetExceptionCode()))
		{
			return false;
		}
	}
#else
	bool guarded_invoke(void (*fn)(void* ctx), void* ctx) noexcept
	{
		fn(ctx);
		return true;
	}
#endif
}
