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
				"LUAF_FREEPROTO",
				"48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 44 0F B6 4A",
				[](const memory::handle ptr) {
					g_dumper_pointers->m_luau_functions.luaF_freeproto = ptr.as<uintptr_t>();
				},
			},
			{
				"TABLE",
				"44 8B 4A ? 48 8B CB",
				[](const memory::handle ptr) {
					g_dumper_pointers->m_luau_functions.table = ptr.as<uintptr_t>();
				},
			},
			{
				"THREAD",
					"48 83 EC ? 8B 42 ? 4C 8D 4A",
					[](const memory::handle ptr) {
						g_dumper_pointers->m_luau_functions.thread = ptr.as<uintptr_t>();
				},
			},
			{
			"GC",
				"44 0F B6 47 ? 45 85 C0",
				[](const memory::handle ptr) {
					g_dumper_pointers->m_luau_functions.gc = ptr.as<uintptr_t>();
				},
			},
			{
				"PAGE",
				"48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F1 49 63 E8",
				[](const memory::handle ptr) {
					g_dumper_pointers->m_luau_functions.page = ptr.as<uintptr_t>();
				},
			},
			{
				"PROPAGATEMARK",
				"48 83 EC ? 48 8B 59 ? 48 8B F9 80 4B",
				[](const memory::handle ptr) {
					g_dumper_pointers->m_luau_functions.propagatemark = ptr.as<uintptr_t>();
				},
			}
        >();
		// clang-format on

		return batch_and_hash;
	}

	pointers::pointers()
	{
		g_dumper_pointers = this;

		const auto roblox_region            = memory::module("RobloxStudioBeta.exe");
		const auto [m_dumper_batch, m_hash] = get_luau_batch();

		constexpr cstxpr_str dumper_batch_name{"dumper"};

		run_batch<dumper_batch_name>(m_dumper_batch, roblox_region);

		if (const auto m_hwnd = GetForegroundWindow(); !m_hwnd)
			throw std::runtime_error("Failed to find Roblox Studio window");
	}

	pointers::~pointers()
	{
		g_dumper_pointers = nullptr;
	}
}
