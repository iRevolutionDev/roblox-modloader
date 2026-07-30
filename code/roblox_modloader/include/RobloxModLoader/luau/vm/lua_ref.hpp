#pragma once

#include "RobloxModLoader/internal/common.hpp"

namespace rml::luau::vm
{
	class Ref final
	{
	public:
		Ref() = default;

		Ref(lua_State* registry_owner, const int id) noexcept
			: m_owner(registry_owner), m_id(id)
		{
		}

		~Ref() { reset(); }

		Ref(const Ref&) = delete;
		Ref& operator=(const Ref&) = delete;

		Ref(Ref&& other) noexcept
			: m_owner(std::exchange(other.m_owner, nullptr)), m_id(std::exchange(other.m_id, LUA_NOREF))
		{
		}

		Ref& operator=(Ref&& other) noexcept
		{
			if (this != &other)
			{
				reset();
				m_owner = std::exchange(other.m_owner, nullptr);
				m_id = std::exchange(other.m_id, LUA_NOREF);
			}
			return *this;
		}

		[[nodiscard]] static Ref take(lua_State* L, int index);

		[[nodiscard]] bool valid() const noexcept { return m_owner != nullptr && m_id != LUA_NOREF; }
		[[nodiscard]] int id() const noexcept { return m_id; }

		bool push(lua_State* L) const;

		void reset() noexcept;

	private:
		lua_State* m_owner{nullptr};
		int m_id{LUA_NOREF};
	};
}
