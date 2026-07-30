#pragma once

#include <Luau/Compiler.h>

namespace rml::luau::vm
{
	inline constexpr Luau::CompileOptions kCompileOptions{
	    .optimizationLevel = 1,
	    .debugLevel = 2,
	};
}
