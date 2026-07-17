#pragma once

namespace rml::platform
{
	[[nodiscard]] bool lock_engine_mutex(void* mutex);
	bool unlock_engine_mutex(void* mutex);
}
