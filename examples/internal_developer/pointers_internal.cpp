#include "RobloxModLoader/common.hpp"
#include "pointers_internal.hpp"
#include "RobloxModLoader/memory/all.hpp"

constexpr auto pointers_internal::get_roblox_batch() {
    // clang-format off
    constexpr auto batch_and_hash = memory::make_batch<
        {
            "IS_INTERNAL",
            "E8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? 84 C0 48 0F 45 CA 48 8D 05 ? ? ? ? 48 89 45 ? 48 C7 45 ? ? ? ? ? 48 89 5D",
            [](const memory::handle ptr) {
                const auto call_offset = ptr.add(1).as<std::int32_t*>();
                const auto target_address = ptr.add(5).add(*call_offset);
                g_pointers_internal->m_roblox_pointers.m_is_internal = target_address.as<PVOID>();

                for (int offset = 0; offset < 0x100; offset++) {
                    if (auto instruction_ptr = target_address.add(offset); instruction_ptr.as<uint8_t*>()[0] == 0x80 &&
                                                                           instruction_ptr.as<uint8_t*>()[1] == 0x3D &&
                                                                           instruction_ptr.as<uint8_t*>()[6] == 0x00) {

                        const auto cmp_address_offset = instruction_ptr.add(2).as<std::int32_t*>();
                        const auto cmp_target_address = instruction_ptr.add(7).add(*cmp_address_offset);
                        g_pointers_internal->m_roblox_pointers.m_is_internal_flag = cmp_target_address.as<uintptr_t>();
                        break;
                    }
                }
            },
        }
    >();

    // clang-format on

    return batch_and_hash;
}

pointers_internal::pointers_internal() {
    g_pointers_internal = this;

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

pointers_internal::~pointers_internal() {
    g_pointers_internal = nullptr;
}
