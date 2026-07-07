#pragma once
#include "interop_registry.hpp"

namespace rml::dotnet
{
	class RobloxInteropProvider
	{
	public:
		static void populate(InteropTable& table);

	private:
		static void verify_populated(const InteropTable& table);
	};
} // namespace rml::dotnet