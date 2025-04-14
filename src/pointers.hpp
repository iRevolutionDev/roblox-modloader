#pragma once
#include "RobloxModLoader/memory/batch.hpp"
#include "RobloxModLoader/memory/byte_patch.hpp"
#include "RobloxModLoader/memory/module.hpp"
#include "RobloxModLoader/util/compile_time_helpers.hpp"
#include "roblox_pointers.hpp"

class pointers
{
private:
	static constexpr auto get_roblox_batch();

    template<cstxpr_str batch_name, size_t N>
    void run_batch(const memory::batch<N>& batch, const memory::module& mem_region)
    {
        if (!memory::batch_runner::run(batch, mem_region))
        {
            const std::string error_message =
                std::string("Failed to find some patterns for ") + std::string(batch_name.str);
            throw std::runtime_error(error_message);
        }
    }

public:
	explicit pointers();
	~pointers();

public:
	HWND m_hwnd{};

public:
	roblox_pointers m_roblox_pointers;
};

inline pointers* g_pointers{};