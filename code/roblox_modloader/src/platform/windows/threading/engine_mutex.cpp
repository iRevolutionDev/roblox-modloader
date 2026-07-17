#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/platform/threading/engine_mutex.hpp"

namespace rml::platform
{
	bool lock_engine_mutex(void* mutex)
	{
		if (!mutex)
			return false;

		_Mtx_lock(static_cast<_Mtx_t>(mutex));
		return true;
	}

	bool unlock_engine_mutex(void* mutex)
	{
		if (!mutex)
			return false;

		_Mtx_unlock(static_cast<_Mtx_t>(mutex));
		return true;
	}
}
