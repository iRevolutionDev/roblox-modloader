#pragma once

#include "RobloxModLoader/memory/all.hpp"
#include "pointers.hpp"

namespace rml
{
	constexpr auto Pointers::get_roblox_batch()
	{
		constexpr auto batch_and_hash = memory::make_batch<>();

		return batch_and_hash;
	}
}
