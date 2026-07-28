#pragma once

#include "rml/dumper/core/types.hpp"

#include <array>
#include <string_view>

namespace rml::dumper::target
{
	enum class Anchor : std::uint8_t
	{
		luaF_newLclosure,
		luaF_newCclosure,
		luaF_newproto,
		luaF_freeproto,
		luaF_findupval,
		luaU_load,
		luaM_free,
		luaD_reallocstack,
		luaD_reallocCI,
		luaH_new,
		setnodevector,
		propagatemark,
		traversetable,
		lua_resume,
		index2addr,
		count,
	};

	inline constexpr std::size_t anchor_count = static_cast<std::size_t>(Anchor::count);

	[[nodiscard]] constexpr std::string_view to_string(const Anchor value)
	{
		switch (value)
		{
		case Anchor::luaF_newLclosure: return "luaF_newLclosure";
		case Anchor::luaF_newCclosure: return "luaF_newCclosure";
		case Anchor::luaF_newproto: return "luaF_newproto";
		case Anchor::luaF_freeproto: return "luaF_freeproto";
		case Anchor::luaF_findupval: return "luaF_findupval";
		case Anchor::luaU_load: return "luaU_load";
		case Anchor::luaM_free: return "luaM_free";
		case Anchor::luaD_reallocstack: return "luaD_reallocstack";
		case Anchor::luaD_reallocCI: return "luaD_reallocCI";
		case Anchor::luaH_new: return "luaH_new";
		case Anchor::setnodevector: return "setnodevector";
		case Anchor::propagatemark: return "propagatemark";
		case Anchor::traversetable: return "traversetable";
		case Anchor::lua_resume: return "lua_resume";
		case Anchor::index2addr: return "index2addr";
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
