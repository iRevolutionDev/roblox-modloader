#include "RobloxModLoader/luau/vm/vm_api.hpp"

#include "pointers.hpp"

RML_LOG_SCOPE("LuauApi");

namespace rml::luau::vm
{
	struct ApiEntry
	{
		std::string_view name;
		bool present;
	};

	static ApiAvailability probe()
	{
		ApiAvailability report;

		if (!g_pointers)
		{
			report.missing.emplace_back("g_pointers");
			return report;
		}

		const auto& p = g_pointers->m_roblox_pointers;

		const std::array entries{
		    ApiEntry{"lua_gettop", p.lua_gettop != nullptr},
		    ApiEntry{"lua_settop", p.lua_settop != nullptr},
		    ApiEntry{"lua_pushvalue", p.lua_pushvalue != nullptr},
		    ApiEntry{"lua_insert", p.lua_insert != nullptr},
		    ApiEntry{"lua_type", p.lua_type != nullptr},
		    ApiEntry{"lua_isnumber", p.lua_isnumber != nullptr},
		    ApiEntry{"lua_isstring", p.lua_isstring != nullptr},
		    ApiEntry{"lua_iscfunction", p.lua_iscfunction != nullptr},
		    ApiEntry{"lua_isuserdata", p.lua_isuserdata != nullptr},
		    ApiEntry{"lua_toboolean", p.lua_toboolean != nullptr},
		    ApiEntry{"lua_tonumberx", p.lua_tonumberx != nullptr},
		    ApiEntry{"lua_tointegerx", p.lua_tointegerx != nullptr},
		    ApiEntry{"lua_tolstring", p.lua_tolstring != nullptr},
		    ApiEntry{"lua_topointer", p.lua_topointer != nullptr},
		    ApiEntry{"lua_objlen", p.lua_objlen != nullptr},
		    ApiEntry{"lua_pushnil", p.lua_pushnil != nullptr},
		    ApiEntry{"lua_pushnumber", p.lua_pushnumber != nullptr},
		    ApiEntry{"lua_pushinteger", p.lua_pushinteger != nullptr},
		    ApiEntry{"lua_pushboolean", p.lua_pushboolean != nullptr},
		    ApiEntry{"lua_pushlstring", p.lua_pushlstring != nullptr},
		    ApiEntry{"lua_pushstring", p.lua_pushstring != nullptr},
		    ApiEntry{"lua_pushcclosurek", p.lua_pushcclosurek != nullptr},
		    ApiEntry{"lua_createtable", p.lua_createtable != nullptr},
		    ApiEntry{"lua_getfield", p.lua_getfield != nullptr},
		    ApiEntry{"lua_setfield", p.lua_setfield != nullptr},
		    ApiEntry{"lua_gettable", p.lua_gettable != nullptr},
		    ApiEntry{"lua_settable", p.lua_settable != nullptr},
		    ApiEntry{"lua_rawget", p.lua_rawget != nullptr},
		    ApiEntry{"lua_rawgeti", p.lua_rawgeti != nullptr},
		    ApiEntry{"lua_rawset", p.lua_rawset != nullptr},
		    ApiEntry{"lua_rawseti", p.lua_rawseti != nullptr},
		    ApiEntry{"lua_next", p.lua_next != nullptr},
		    ApiEntry{"lua_getmetatable", p.lua_getmetatable != nullptr},
		    ApiEntry{"lua_setmetatable", p.lua_setmetatable != nullptr},
		    ApiEntry{"lua_getreadonly", p.lua_getreadonly != nullptr},
		    ApiEntry{"lua_setreadonly", p.lua_setreadonly != nullptr},
		    ApiEntry{"lua_getupvalue", p.lua_getupvalue != nullptr},
		    ApiEntry{"lua_setupvalue", p.lua_setupvalue != nullptr},
		    ApiEntry{"lua_ref", p.lua_ref != nullptr},
		    ApiEntry{"lua_unref", p.lua_unref != nullptr},
		    ApiEntry{"lua_pcall", p.lua_pcall != nullptr},
		    ApiEntry{"lua_call", p.lua_call != nullptr},
		    ApiEntry{"lua_resume", p.lua_resume != nullptr},
		    ApiEntry{"lua_yield", p.lua_yield != nullptr},
		    ApiEntry{"lua_newthread", p.lua_newthread != nullptr},
		    ApiEntry{"lua_getinfo", p.lua_getinfo != nullptr},
		    ApiEntry{"lua_error", p.lua_error != nullptr},
		    ApiEntry{"luaL_register", p.luaL_register != nullptr},
		    ApiEntry{"luaL_where", p.luaL_where != nullptr},
		    ApiEntry{"luaL_checkinteger", p.luaL_checkinteger != nullptr},
		    ApiEntry{"luaL_checknumber", p.luaL_checknumber != nullptr},
		    ApiEntry{"luaL_checklstring", p.luaL_checklstring != nullptr},
		    ApiEntry{"luaL_checktype", p.luaL_checktype != nullptr},
		    ApiEntry{"luaL_checkany", p.luaL_checkany != nullptr},
		    ApiEntry{"luaL_optinteger", p.luaL_optinteger != nullptr},
		    ApiEntry{"luaL_optboolean", p.luaL_optboolean != nullptr},
		    ApiEntry{"luaL_sandboxthread", p.luaL_sandboxthread != nullptr},
		    ApiEntry{"luaL_typeerrorL", p.luaL_typeerrorL != nullptr},
		    ApiEntry{"luaL_argerrorL", p.luaL_argerrorL != nullptr},
		    ApiEntry{"luaF_newLclosure", p.luaF_newLclosure != nullptr},
		    ApiEntry{"luaC_barrierf", p.luaC_barrierf != nullptr},
		    ApiEntry{"luaC_barrierback", p.luaC_barrierback != nullptr},
		    ApiEntry{"luaC_barriertable", p.luaC_barriertable != nullptr},
		    ApiEntry{"luaH_setnum", p.luaH_setnum != nullptr},
		    ApiEntry{"luaA_pseudo2addr", p.luaA_pseudo2addr != nullptr},
		    ApiEntry{"luaO_nilobject", p.luaO_nilobject != nullptr},
		    ApiEntry{"luau_load", p.luau_load != nullptr},
		    ApiEntry{"get_global_state", p.get_global_state != nullptr},
		};

		for (const auto& [name, present] : entries)
		{
			if (!present)
			{
				report.missing.push_back(name);
			}
		}

		report.ready = report.missing.empty();
		return report;
	}

	const ApiAvailability& api_availability() noexcept
	{
		static const ApiAvailability report = probe();
		return report;
	}

	void report_api_unavailable_once() noexcept
	{
		static std::once_flag reported;
		std::call_once(reported, [] {
			const auto& report = api_availability();
			if (report.ready)
			{
				return;
			}

			std::string names;
			for (const auto& name : report.missing)
			{
				if (!names.empty())
				{
					names.append(", ");
				}
				names.append(name);
			}

			RML_WARN("Luau engine functions unavailable for this Studio build - mod scripts disabled "
			         "(signatures will re-scan on the next build); missing: {}",
			         names);
		});
	}
}
