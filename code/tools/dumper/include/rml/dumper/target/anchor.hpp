#pragma once

#include "rml/dumper/core/types.hpp"

#include <array>
#include <string_view>

namespace rml::dumper::target
{
	enum class Anchor : std::uint8_t
	{
		luaD_reallocstack,
		luaD_reallocCI,
		luaE_newthread,
		lua_settop,
		lua_resume,
		luaM_free,
		luaF_findupval,
		luaH_new,
		luaF_newLclosure,
		luaF_newCclosure,
		luaF_freeproto,
		luaU_load,
		lua_pushnumber,
		lua_toboolean,
		count,
	};

	inline constexpr std::size_t anchor_count = static_cast<std::size_t>(Anchor::count);

	[[nodiscard]] constexpr std::string_view to_string(const Anchor value)
	{
		switch (value)
		{
		case Anchor::luaD_reallocstack: return "luaD_reallocstack";
		case Anchor::luaD_reallocCI: return "luaD_reallocCI";
		case Anchor::luaE_newthread: return "luaE_newthread";
		case Anchor::lua_settop: return "lua_settop";
		case Anchor::lua_resume: return "lua_resume";
		case Anchor::luaM_free: return "luaM_free";
		case Anchor::luaF_findupval: return "luaF_findupval";
		case Anchor::luaH_new: return "luaH_new";
		case Anchor::luaF_newLclosure: return "luaF_newLclosure";
		case Anchor::luaF_newCclosure: return "luaF_newCclosure";
		case Anchor::luaF_freeproto: return "luaF_freeproto";
		case Anchor::luaU_load: return "luaU_load";
		case Anchor::lua_pushnumber: return "lua_pushnumber";
		case Anchor::lua_toboolean: return "lua_toboolean";
		case Anchor::count: return "count";
		}
		return "unknown";
	}

	struct AnchorSpec
	{
		Anchor id{};
		std::string_view pattern;
	};

	class AnchorSet
	{
	public:
		[[nodiscard]] Rva at(const Anchor id) const { return m_addresses[static_cast<std::size_t>(id)]; }
		[[nodiscard]] bool has(const Anchor id) const { return m_addresses[static_cast<std::size_t>(id)] != 0; }

		void set(const Anchor id, const Rva address) { m_addresses[static_cast<std::size_t>(id)] = address; }

	private:
		std::array<Rva, anchor_count> m_addresses{};
	};
}
