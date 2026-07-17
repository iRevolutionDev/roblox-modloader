#include "RobloxModLoader/platform/threading/engine_mutex.hpp"

namespace rml::platform
{
	bool lock_engine_mutex(void*)
	{
		return false;
	}

	bool unlock_engine_mutex(void*)
	{
		return false;
	}
}
