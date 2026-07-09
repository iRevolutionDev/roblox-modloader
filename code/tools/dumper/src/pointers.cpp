#include "dumper/pointers.hpp"

#include "dumper/memory/module.hpp"

#include <stdexcept>

namespace dumper
{
	constexpr auto pointers::get_luau_batch()
	{
		// clang-format off
		constexpr auto batch_and_hash = memory::make_batch<
			{
				"LUAF_NEWLCLOSURE",
				"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B EA 49 8B F8",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaF_newLclosure = ptr.as<uintptr_t>(); },
			},
			{
				"LUAF_NEWCCLOSURE",
				"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 63 F2 49 8B F8",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaF_newCclosure = ptr.as<uintptr_t>(); },
			},
			{
				"LUAF_NEWPROTO",
				"40 53 48 83 EC 20 44 0F B6 41 ? BA C0 00 00 00",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaF_newproto = ptr.as<uintptr_t>(); },
			},
			{
				"LUAF_FREEPROTO",
				"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 44 0F B6 4A ? 49 8B F0",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaF_freeproto = ptr.as<uintptr_t>(); },
			},
			{
				"LUAF_FINDUPVAL",
				"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 41 ? 48 8D 59",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaF_findupval = ptr.as<uintptr_t>(); },
			},
			{
				"LUAU_LOAD",
				"48 89 5C 24 ? 4C 89 44 24 ? 48 89 54 24 ? 48 89 4C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC E0 01 00 00",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaU_load = ptr.as<uintptr_t>(); },
			},
			{
				"LUAM_FREE",
				"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 59 ? 49 8D 40",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaM_free = ptr.as<uintptr_t>(); },
			},
			{
				"LUAD_REALLOCSTACK",
				"48 89 5C 24 ? 55 48 83 EC 30 48 63 EA",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaD_reallocstack = ptr.as<uintptr_t>(); },
			},
			{
				"LUAD_REALLOCCI",
				"48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 30 48 8B 69 ? 48 8B F9",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaD_reallocCI = ptr.as<uintptr_t>(); },
			},
			{
				"LUAH_NEW",
				"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 41 8B F0 8B EA 44 0F B6 41",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.luaH_new = ptr.as<uintptr_t>(); },
			},
			{
				"SETNODEVECTOR",
				"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 20 41 8B E8",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.setnodevector = ptr.as<uintptr_t>(); },
			},
			{
				"PROPAGATEMARK",
				"48 89 5C 24 ? 57 48 83 EC 20 48 8B 59 ? 48 8B F9 0F B6 03",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.propagatemark = ptr.as<uintptr_t>(); },
			},
			{
				"TRAVERSETABLE",
				"48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC 20 45 33 E4 48 8B F2",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.traversetable = ptr.as<uintptr_t>(); },
			},
			{
				"LUA_RESUME",
				"48 89 5C 24 ? 57 48 83 EC 20 0F B6 41 ? 48 8B F9 3C 01",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.lua_resume = ptr.as<uintptr_t>(); },
			},
			{
				"INDEX2ADDR",
				"48 89 5C 24 ? 57 48 83 EC 20 F6 41 ? ? 48 8B D9 48 63 FA 74 ? 4C 8D 41",
				[](const memory::handle ptr) { g_dumper_pointers->m_luau_functions.index2addr = ptr.as<uintptr_t>(); },
			}
        >();
		// clang-format on

		return batch_and_hash;
	}

	pointers::pointers(const memory::module& region)
	{
		g_dumper_pointers = this;

		const auto [m_dumper_batch, m_hash] = get_luau_batch();

		constexpr cstxpr_str dumper_batch_name{"dumper"};

		run_batch<dumper_batch_name>(m_dumper_batch, region);
	}

	pointers::~pointers()
	{
		g_dumper_pointers = nullptr;
	}
}
