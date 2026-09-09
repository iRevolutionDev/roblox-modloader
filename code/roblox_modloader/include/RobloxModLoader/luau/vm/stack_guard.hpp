#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau::vm
{
	class StackGuard final
	{
	public:
		explicit StackGuard(lua_State* L) noexcept
			: m_state(L), m_top(lua_gettop(L))
		{
		}

		~StackGuard()
		{
			if (m_state)
			{
				lua_settop(m_state, m_top);
			}
		}

		StackGuard(const StackGuard&) = delete;
		StackGuard& operator=(const StackGuard&) = delete;
		StackGuard(StackGuard&&) = delete;
		StackGuard& operator=(StackGuard&&) = delete;

		void release() noexcept { m_state = nullptr; }

		[[nodiscard]] int top() const noexcept { return m_top; }

	private:
		lua_State* m_state;
		int m_top;
	};
}
