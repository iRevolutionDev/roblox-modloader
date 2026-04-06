#pragma once
#include "interop_registry.hpp"

namespace rml::dotnet
{
	class RobloxInteropProvider
	{
	public:
		static void populate(InteropTable& table);
	};
} // namespace rml::dotnet