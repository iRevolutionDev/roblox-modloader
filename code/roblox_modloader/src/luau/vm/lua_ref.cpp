#include "RobloxModLoader/luau/vm/lua_ref.hpp"

namespace rml::luau::vm
{
	Ref Ref::take(lua_State* L, const int index)
	{
		if (!L)
		{
			return {};
		}

		const int id = lua_ref(L, index);
		if (id == LUA_NOREF)
		{
			return {};
		}

		return Ref{L, id};
	}

	bool Ref::push(lua_State* L) const
	{
		if (!L)
		{
			return false;
		}

		if (!valid())
		{
			lua_pushnil(L);
			return false;
		}

		lua_rawgeti(L, LUA_REGISTRYINDEX, m_id);
		return lua_type(L, -1) != LUA_TNIL;
	}

	void Ref::reset() noexcept
	{
		if (m_owner && m_id != LUA_NOREF)
		{
			lua_unref(m_owner, m_id);
		}

		m_owner = nullptr;
		m_id = LUA_NOREF;
	}
}
