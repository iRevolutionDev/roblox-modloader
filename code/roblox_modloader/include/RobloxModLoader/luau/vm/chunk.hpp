#pragma once

#include "RobloxModLoader/luau/vm/vm_error.hpp"

namespace rml::luau::vm
{
	[[nodiscard]] std::expected<void, VmError> load_chunk(lua_State* L, std::string_view chunk_name,
	                                                      std::span<const std::byte> bytecode,
	                                                      std::uint64_t capabilities, int env = 0);
}
