#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/roblox/security/script_permissions.hpp"

struct Closure;

namespace rml::luau::vm
{
	bool set_identity(lua_State* L, RBX::Security::Permissions identity, std::uint64_t capabilities,
	                  bool reflect_identity_number = true) noexcept;

	void elevate_closure(const Closure* closure, std::uint64_t capabilities) noexcept;

	void elevate_stack_closure(lua_State* L, int index, std::uint64_t capabilities) noexcept;
}
