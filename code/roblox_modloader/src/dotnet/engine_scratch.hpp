#pragma once

#include <cstddef>

namespace rml::dotnet
{
	template<std::size_t N>
	struct EngineScratch
	{
		std::byte storage[N]{};
	};
} // namespace rml::dotnet
