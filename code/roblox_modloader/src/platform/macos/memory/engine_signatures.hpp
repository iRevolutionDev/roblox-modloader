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
			}
		>();
		// clang-format on

		return batch_and_hash;
	}
}
