#include "pointers.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/all.hpp"

namespace rml
{
	constexpr auto Pointers::get_roblox_batch()
	{
		// clang-format off
	    constexpr auto batch_and_hash = memory::make_batch<
	         // Lua Functions
	         {
	             "LUA_LOAD",
	             "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 81 EC ? ? ? ? 49 8B E9 4D 8B F0 4C 8B FA 48 8B F9",
	             [](const memory::handle ptr) {
	                 g_pointers->m_roblox_pointers.luau_load = ptr.as<functions::luau_load>();
	             },
	         },
	        {
	            "LUAU_EXECUTE",
	            "80 79 ? ? 0F 85 ? ? ? ? E9 ? ? ? ? CC",
	            [](const memory::handle ptr) {
	                g_pointers->m_roblox_pointers.luau_execute = ptr.as<functions::luau_execute>();
	            },
	        },
	         {
	             "LUAE_NEWTHREAD",
	             "48 89 5C 24 ? 57 48 83 EC ? 44 0F B6 41 ? BA",
	             [](const memory::handle ptr) {
	                 g_pointers->m_roblox_pointers.luaE_newthread = ptr.as<functions::luaE_newthread>();
	             },
	         },
	         {
	             "LUAH_NEW",
	             "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F0 8B EA 44 0F B6 41",
	             [](const memory::handle ptr) {
	                 g_pointers->m_roblox_pointers.luaH_new = ptr.as<functions::luaH_new>();
	             },
	         },
	         {
	             "FREEBLOCK",
	             "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57",
	             [](const memory::handle ptr) {
	                 g_pointers->m_roblox_pointers.freeblock = ptr.as<functions::freeblock>();
	             },
	         },
	        {
	                "LUAD_RAWRUNPROTECTED",
	                "48 89 4C 24 ? 48 83 EC ? 48 8B C2 49 8B D0 FF D0 33 C0 EB 04 8B 44 24 48 48 83 C4 ? C3",
	                [](const memory::handle ptr) {
	                    g_pointers->m_roblox_pointers.luaD_rawrunprotected = ptr.as<functions::luaD_rawrunprotected>();
	                }
	            },
	        {
	            "LUAD_THROW",
	                "48 83 EC ? 44 8B C2 48 8B D1 48 8D 4C 24",
	                [](const memory::handle ptr) {
	                    g_pointers->m_roblox_pointers.luaD_throw = ptr.as<functions::luaD_throw>();
	                }
	        },
	        {
	            "LUA_SETFIELD",
	                "48 89 5C 24 ? 57 48 83 EC ? 4D 8B D0 48 8B F9 85 D2 7E ? 4C 8B 49 ? 48 8D 1D ? ? ? ? 49 83 C1 ? 48 63 D2 48 C1 E2 ? 4C 03 CA 4C 3B 49 ? 49 0F 42 D9 EB ? 81 FA ? ? ? ? 7E ? 48 63 DA 48 C1 E3 ? 48 03 59 ? EB ? E8 ? ? ? ? 48 8B D8 49 C7 C0",
	                [](const memory::handle ptr) {
	                    g_pointers->m_roblox_pointers.lua_setfield = ptr.as<functions::lua_setfield>();
	            }
	        },
	        {
	            "PROFILE_LOG",
	            "40 55 56 57 41 56 48 83 EC ? 48 8B 05",
	            [](const memory::handle ptr) {
	                g_pointers->m_roblox_pointers.m_profile_log = ptr.as<PVOID>();
	            },
	        },
	        {
	            "OBJECT_CREATE_BY_NAME",
	            "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 41 8B F9 48 8B EA",
	            [](const memory::handle ptr) {
	                g_pointers->m_roblox_pointers.object_create_by_name = ptr.as<functions::object_create_by_name>();
	            },
	        },
	        {
	            "INSTANCE_BRIDGE_PUSH",
	            "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 E8 ? ? ? ? 48 8B CB 84 C0 74 ? 48 8B D7",
	            [](const memory::handle ptr) {
	                g_pointers->m_roblox_pointers.instance_bridge_push = ptr.as<functions::instance_bridge_push>();
	            },
	        },
			{
				"DESCRIPTOR_LOOKUP",
				"48 83 EC 18 ? ? ? 4C 8B D9 75",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.descriptor_lookup = ptr.as<functions::descriptor_lookup>();
				}
			},
			{
				"GET_STRING_ATOM",
				"48 89 5C 24 ? 57 48 83 EC 20 48 8B 1D ? ? ? ? 48 8B F9 48 85 DB",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.get_string_atom = ptr.as<functions::get_string_atom>();
				}
			},
			{
				"MENU_BUILD_FROM_DOM",
				"48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC 50 01 00 00 49 8B D8",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.build_menu_bar_from_dom = ptr.as<functions::build_menu_bar_from_dom>();
				}
			}
	    >();

		// clang-format on

		return batch_and_hash;
	}

	Pointers::Pointers()
	{
		g_pointers = this;

		const auto roblox_region = memory::module(std::string_view{"RobloxStudioBeta.exe"});
		const auto [m_roblox_batch, m_hash] = get_roblox_batch();

		constexpr cstxpr_str roblox_batch_name{"roblox"};

		run_batch<roblox_batch_name>(m_roblox_batch, roblox_region);

		m_hwnd = GetForegroundWindow();

		if (!m_hwnd)
			throw std::runtime_error("Failed to find Roblox Studio window");
	}

	Pointers::~Pointers()
	{
		g_pointers = nullptr;
	}
}

RobloxPointers* get_roblox_pointers()
{
	if (!g_pointers)
	{
		throw std::runtime_error("Pointers not initialized");
	}

	return &g_pointers->m_roblox_pointers;
}
