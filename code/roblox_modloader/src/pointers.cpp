#include "pointers.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/memory/all.hpp"

constexpr auto pointers::get_roblox_batch()
{
	// clang-format off
    constexpr auto batch_and_hash = memory::make_batch<
        // {
        //     "RBXCRASH",
        //     "48 89 5C 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B FA 48 8B D9 48 8B 05 ? ? ? ? 48 85 C0",
        //     [](const memory::handle ptr) {
        //         g_pointers->m_roblox_pointers.m_rbx_crash = ptr.as<PVOID>();
        //     },
        // },
        // Render Perform
         {
             "RP",
             "48 8B C4 48 89 58 ? 44 89 48 ? 4C 89 40 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 70 ? 0F 29 78",
             [](const memory::handle ptr) {
                 g_pointers->m_roblox_pointers.m_render_perform = ptr.as<PVOID>();
             },
         },
         // Render Prepare
         {
             "RPR",
             "48 8B C4 44 89 48 ?? 44 88 40 ?? 48 89 50",
             [](const memory::handle ptr) {
                 g_pointers->m_roblox_pointers.m_render_prepare = ptr.as<PVOID>();
             },
         },
         // Scene Render View
         {
             "SRV",
             "48 89 5C 24 ?? 4C 89 4C 24 ?? 4C 89 44 24 ?? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ?? ?? ?? ?? B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 2B E0 0F 29 B4 24",
             [](const memory::handle ptr) {
                 g_pointers->m_roblox_pointers.m_render_view = ptr.as<PVOID>();
             },
         },
         // {
         //     "GET_SCHEDULER",
         //     "40 53 48 83 EC ? BB ? ? ? ? E8 ? ? ? ? 8B 0D ? ? ? ? 84 C0 65 48 8B 04 25 ? ? ? ? 48 8B 0C C8 8B 04 0B 74 ? 39 05 ? ? ? ? 7E ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 83 3D ? ? ? ? ? 75 ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 05 ? ? ? ? 0F B6 40",
         //     [](const memory::handle ptr) {
         //         g_pointers->m_roblox_pointers.get_scheduler = ptr.as<functions::get_scheduler>();
         //     },
         // },
        // {
        //     "PRINT",
        //     "48 8B C4 48 89 50 ? 4C 89 40 ? 4C 89 48 ? 53 48 83 EC ? 8B D9",
        //     [](const memory::handle ptr) {
        //         g_pointers->m_roblox_pointers.print = ptr.as<functions::print>();
        //     },
        // },
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
         // {
         //     "LUA_PUSHVALUE",
         //     "48 89 5C 24 ? 57 48 83 EC ? F6 41 ? ? 48 8B D9 48 63 FA 74 ? 4C 8D 41 ? 48 8B D1 E8 ? ? ? ? 85 FF 7E ? 48 8B 43 ? 48 8B CF 48 C1 E1",
         //     [](const memory::handle ptr) {
         //         g_pointers->m_roblox_pointers.lua_pushvalue = ptr.as<functions::lua_pushvalue>();
         //     },
         // },
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
         // {
         //     "LUA_NEWTHREAD",
         //     "48 89 5C 24 ? 57 48 83 EC ? 48 8B 51 ? 48 8B D9 48 8B 42",
         //     [](const memory::handle ptr) {
         //         g_pointers->m_roblox_pointers.lua_newthread = ptr.as<functions::lua_newthread>();
         //     }
         // },
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
        // {
        //     "GET_GLOBALSTATE",
        //     "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B F8 48 8B F2 48 8B D9 8B 81 ? ? ? ? 90 83 F8 ? 7C ? 48 8D 05 ? ? ? ? 48 89 44 24 ? 48 8B 54 24 ? 48 81 EA ? ? ? ? 33 C9 E8 ? ? ? ? 90 48 8D 8B ? ? ? ? 4C 8B C7 48 8B D6 E8 ? ? ? ? 48 83 C0 ? ? ? 03 D0 89 54 24 ? 03 40 ? 89 44 24 ? 48 8B 44 24 ? 48 8B 5C 24 ? 48 8B 74 24 ? 48 83 C4 ? 5F C3 ? ? 40 53",
        //     [](const memory::handle ptr) {
        //         g_pointers->m_roblox_pointers.get_global_state = ptr.as<functions::get_global_state>();
        //     }
        // },
        // {
        //     "TASK_DEFER",
        //     "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F1 33 FF 40 38 3D",
        //     [](const memory::handle ptr) {
        //         g_pointers->m_roblox_pointers.task_defer = ptr.as<functions::task_defer>();
        //     },
        // },
        {
        "RESUME_WAITING_SCRIPS",
            "48 89 4C 24 ? 53 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 4C 8B E9",
            [](const memory::handle ptr) {
                g_pointers->m_roblox_pointers.resume_waiting_scripts = ptr.as<PVOID>();
            },
        },
        {
            "PROFILE_LOG",
            "40 55 56 57 41 56 48 83 EC ? 48 8B 05",
            [](const memory::handle ptr) {
                g_pointers->m_roblox_pointers.m_profile_log = ptr.as<PVOID>();
            },
        },
        // {
        //     "CREATOR_CREATE_BY_NAME",
        //     "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 41 8B F8 48 8B F1",
        //     [](const memory::handle ptr) {
        //         g_pointers->m_roblox_pointers.creator_create_by_name = ptr.as<functions::creator_create_by_name>();
        //     },
        // },
        {
            "INSTANCE_BRIDGE_PUSH",
            "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 E8 ? ? ? ? 48 8B CB 84 C0 74 ? 48 8B D7",
            [](const memory::handle ptr) {
                g_pointers->m_roblox_pointers.instance_bridge_push = ptr.as<functions::instance_bridge_push>();
            },
        }
    >();

	// clang-format on

	return batch_and_hash;
}

pointers::pointers()
{
	g_pointers = this;

	const auto roblox_region            = memory::module("RobloxStudioBeta.exe");
	const auto [m_roblox_batch, m_hash] = get_roblox_batch();

	constexpr cstxpr_str roblox_batch_name{"roblox"};

	run_batch<roblox_batch_name>(m_roblox_batch, roblox_region);

	m_hwnd = GetForegroundWindow();

	if (!m_hwnd)
		throw std::runtime_error("Failed to find Roblox Studio window");
}

pointers::~pointers()
{
	g_pointers = nullptr;
}

roblox_pointers* get_roblox_pointers()
{
	if (!g_pointers)
	{
		throw std::runtime_error("Pointers not initialized");
	}

	return &g_pointers->m_roblox_pointers;
}
