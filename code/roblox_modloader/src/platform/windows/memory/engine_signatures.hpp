#pragma once

#include "RobloxModLoader/memory/all.hpp"
#include "pointers.hpp"

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
	                g_pointers->m_roblox_pointers.m_profile_log = ptr.as<void*>();
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
			},
			{
				"SIGNAL_DISCONNECT",
				"48 89 5C 24 ? 57 48 83 EC 30 48 8B F9 33 DB 48 89 5C 24 ? E8 ? ? ? ? 48 89 44 24 ? 88 5C 24 ? 48 8B C8 E8 ? ? ? ? 85 C0 0F 85",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.signal_disconnect = ptr.as<functions::signal_disconnect>();
					g_pointers->m_roblox_pointers.signal_mutex_get = ptr.add(21).rip().as<functions::signal_mutex_get>();
				}
			},
			{
				"SIGNAL_SLOT_FREE",
				"48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8D 50 10 BF FF FF FF FF 48 3B DA 0F 82",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.signal_slot_free = ptr.as<functions::signal_slot_free>();
				}
			},
			{
				"TYPE_REGISTRY",
				"48 8B 15 ? ? ? ? 48 8D 0D ? ? ? ? 48 3B 15 ? ? ? ? 48 89 0D ? ? ? ? 48 89 74 24",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.type_registry = ptr.add(10).rip().as<const std::vector<const RBX::Reflection::Type*>*>();
				}
			}
	    >();

		// clang-format on

		return batch_and_hash;
	}
}
