#pragma once

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/roblox/security/script_permissions.hpp"

struct Closure;
struct Proto;

namespace rml::luau::vm
{
	bool set_identity(lua_State* L, RBX::Security::Permissions identity, std::uint64_t capabilities) noexcept;

	class IdentityScope final
	{
	public:
		IdentityScope(lua_State* L, RBX::Security::Permissions identity, std::uint64_t capabilities) noexcept;

		~IdentityScope();

		IdentityScope(const IdentityScope&) = delete;
		IdentityScope& operator=(const IdentityScope&) = delete;
		IdentityScope(IdentityScope&&) = delete;
		IdentityScope& operator=(IdentityScope&&) = delete;

	private:
		lua_State* m_state;
		RBX::Security::Permissions m_previous_identity{};
		std::uint64_t m_previous_capabilities{};
		bool m_restore{false};
	};

	void elevate_closure(const Closure* closure, std::uint64_t capabilities) noexcept;
}
