#include "RobloxModLoader/luau/vm/chunk.hpp"

#include "RobloxModLoader/luau/vm/thread_identity.hpp"

#include "pointers.hpp"

RML_LOG_SCOPE("LuauChunk");

namespace rml::luau::vm
{
	std::expected<void, VmError> load_chunk(lua_State* L, const std::string_view chunk_name,
	                                        const std::span<const std::byte> bytecode,
	                                        const std::uint64_t capabilities, const int env)
	{
		const auto load = g_pointers ? g_pointers->m_roblox_pointers.luau_load : nullptr;
		if (load == nullptr)
		{
			return std::unexpected(VmError::unavailable("luau_load is not resolved on this Studio build"));
		}

		const auto name = std::format("={}", chunk_name);

		if (load(L, name.c_str(), reinterpret_cast<const char*>(bytecode.data()), bytecode.size(), env) != LUA_OK)
		{
			return std::unexpected(error_from_stack(L, VmError::Kind::Syntax));
		}

		elevate_stack_closure(L, -1, capabilities);
		return {};
	}
}
