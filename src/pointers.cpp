#include "RobloxModLoader/common.hpp"
#include "pointers.hpp"
#include "RobloxModLoader/memory/all.hpp"

constexpr auto pointers::get_roblox_batch() {
    // clang-format off
    constexpr auto batch_and_hash = memory::make_batch<
        {
            "RBXCRASH",
            "48 89 5C 24 ? 48 89 7C 24 ? 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B FA 48 8B D9 48 8B 05 ? ? ? ? 48 85 C0",
            [](const memory::handle ptr) {
                g_pointers->m_roblox_pointers.m_rbx_crash = ptr.as<PVOID>();
            },
        },
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
    {
        // On Window Create
        "OWC",
            "48 89 5C 24 ?? 48 89 74 24 ?? 48 89 4C 24 ?? 57 41 54 41 55 41 56 41 57 48 81 EC ?? ?? ?? ?? 49 8B D1",
            [](const memory::handle ptr) {
                g_pointers->m_roblox_pointers.m_on_window_create = ptr.as<PVOID>();
            },
    }
    >();

    // clang-format on

    return batch_and_hash;
}

pointers::pointers() {
    g_pointers = this;

    const auto roblox_region = memory::module("RobloxStudioBeta.exe");
    const auto [m_roblox_batch, m_hash] = get_roblox_batch();

    constexpr cstxpr_str roblox_batch_name{"roblox"};

    run_batch<roblox_batch_name>(
        m_roblox_batch,
        roblox_region
    );

    m_hwnd = GetForegroundWindow();

    if (!m_hwnd)
        throw std::runtime_error("Failed to find Roblox Studio window");
}

pointers::~pointers() {
    g_pointers = nullptr;
}
