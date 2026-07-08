#pragma once
#include "RobloxModLoader/memory/batch.hpp"
#include "RobloxModLoader/memory/module.hpp"
#include "RobloxModLoader/util/compile_time_helpers.hpp"
#include "roblox_pointers_internal.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>

class pointers_internal
{
private:
	static constexpr auto get_roblox_batch();

	template<cstxpr_str batch_name, size_t N>
	void run_batch(const rml::memory::batch<N>& batch, const rml::memory::module& mem_region)
	{
		if (!rml::memory::batch_runner::run(batch, mem_region))
		{
			const std::string error_message = std::string("Failed to find some patterns for ") + std::string(batch_name.str);
			throw std::runtime_error(error_message);
		}
	}

public:
	explicit pointers_internal();

	~pointers_internal();

public:
	HWND m_hwnd{};

public:
	roblox_pointers_internal m_roblox_pointers;
};

inline pointers_internal* g_pointers_internal{};
