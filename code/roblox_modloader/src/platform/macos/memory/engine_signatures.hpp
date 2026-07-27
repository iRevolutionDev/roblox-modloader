#pragma once

#include "RobloxModLoader/memory/all.hpp"
#include "pointers.hpp"

namespace rml
{
	constexpr auto Pointers::get_roblox_batch()
	{
		// clang-format off
		constexpr auto batch_and_hash = memory::make_batch<
			{
				"MENU_BUILD_FROM_DOM",
				"? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 F4 03 01 AA F3 03 00 AA ? ? ? F9 ? ? ? 91 ? ? ? 94",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.build_menu_bar_from_dom = ptr.as<functions::build_menu_bar_from_dom>();
				},
			},
			{
				"GET_STRING_ATOM",
				"? ? ? A9 ? ? ? A9 ? ? ? 91 F3 03 00 AA ? ? ? F0 ? ? ? F9 ? ? ? B5 ? ? ? 97 E1 03 13 AA ? ? ? A9 ? ? ? A8 ? ? ? 14 ? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? 91",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.get_string_atom = ptr.as<functions::get_string_atom>();
				},
			},
			{
				"DESCRIPTOR_LOOKUP",
				"? ? ? B9 ? ? ? 34 ? ? ? A9 ? ? ? A9 ? ? ? 91 F3 03 00 AA ? ? ? 52 ? ? ? 94 ? ? ? B7 ? ? ? A9 ? ? ? B8 ? ? ? B9 49 01 09 0A 00 51 29 8B ? ? ? 14 ? ? ? D2 C0 03 5F D6 ? ? ? D2 ? ? ? A9 ? ? ? A8 C0 03 5F D6 ? ? ? F9",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.descriptor_lookup = ptr.as<functions::descriptor_lookup>();
				},
			},
			{
				"OBJECT_CREATE_BY_NAME",
				"? ? ? D1 ? ? ? A9 ? ? ? A9 ? ? ? A9 ? ? ? 91 F4 03 02 AA F5 03 00 AA F3 03 08 AA E0 03 01 AA ? ? ? 95",
				[](const memory::handle ptr) {
					g_pointers->m_roblox_pointers.object_create_by_name = ptr.as<functions::object_create_by_name>();
				},
			}
		>();
		// clang-format on

		return batch_and_hash;
	}
}
