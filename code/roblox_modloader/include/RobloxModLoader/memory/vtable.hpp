#pragma once

#include <cstdint>

namespace rml::memory
{
	template<typename Pmf>
	[[nodiscard]] std::size_t virtual_index(Pmf pmf)
	{
		const auto* thunk = *reinterpret_cast<const unsigned char* const*>(&pmf);
		if (!thunk || thunk[0] != 0x48 || thunk[1] != 0x8B || thunk[2] != 0x01)
			return static_cast<std::size_t>(-1);

		const unsigned char* jmp = thunk + 3;
		if (jmp[0] != 0xFF)
			return static_cast<std::size_t>(-1);

		switch (jmp[1])
		{
		case 0x20: return 0;
		case 0x60: return jmp[2] / sizeof(void*);
		case 0xA0: return *reinterpret_cast<const std::uint32_t*>(jmp + 2) / sizeof(void*);
		default: return static_cast<std::size_t>(-1);
		}
	}
}
