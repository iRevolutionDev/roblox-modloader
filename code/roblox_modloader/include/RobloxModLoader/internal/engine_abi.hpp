#pragma once

#include "RobloxModLoader/internal/platform.hpp"

#if defined(RML_WINDOWS)
	#define RML_ENGINE_CALL __fastcall
#else
	#define RML_ENGINE_CALL
#endif
