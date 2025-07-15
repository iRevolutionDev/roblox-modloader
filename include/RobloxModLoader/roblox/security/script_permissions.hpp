#pragma once

namespace RBX::Security {
    enum Capabilities : std::uint64_t {
        Plugin = 0x1,
        LocalUser = 0x2,
        WritePlayer = 0x4,
        RobloxScript = 0x8,
        RobloxEngine = 0x10,
        NotAccessible = 0x20,

        RunClientScript = 0x80,
        RunServerScript = 0x100,
        AccessOutsideWrite = 0x400,

        Unassigned = 0x4000,
        AssetRequire = 0x8000,
        LoadStringCapability = 0x10000,
        ScriptGlobals = 0x20000,
        CreateInstances = 0x40000,
        Basic = 0x80000,
        Audio = 0x100000,
        DataStore = 0x200000,
        Network = 0x400000,
        Physics = 0x800000,
        UI = 0x1000000,
        CSG = 0x2000000,
        Chat = 0x4000000,
        Animation = 0x8000000,
        Avatar = 0x10000000,
        RemoteEvent = 0x20000000,
        LegacySound = 0x40000000,

        PluginOrOpenCloud = 0x8000000000000000ull,
        Assistant = 0x4000000000000000ull,

        Restricted = 0xffffffffffffffff
    };

    constexpr std::uint64_t CAPABILITIES_BASE = 0x3FFFFFF00ull;

    constexpr std::uint64_t FULL_CAPABILITIES =
            CAPABILITIES_BASE |
            Plugin | LocalUser | WritePlayer | RobloxScript | RobloxEngine | NotAccessible |
            RunClientScript | RunServerScript | AccessOutsideWrite | Unassigned | AssetRequire |
            LoadStringCapability | ScriptGlobals | CreateInstances | Basic | Audio | DataStore |
            Network | Physics | UI | CSG | Chat | Animation | Avatar | RemoteEvent | LegacySound |
            PluginOrOpenCloud | Assistant;

    constexpr std::uint64_t BASIC_SCRIPT_CAPABILITIES =
            CAPABILITIES_BASE | Plugin | LocalUser | RobloxScript | RunClientScript | RunServerScript | Basic;

    constexpr std::uint64_t PLUGIN_CAPABILITIES =
            CAPABILITIES_BASE | Plugin | LocalUser | WritePlayer | RobloxEngine | CreateInstances | Basic |
            Audio | Network | Physics | UI | CSG | Chat | Animation | Avatar;

    enum class Permissions : std::uint32_t {
        None = 0,
        Plugin = 1,
        RobloxPlace = 2,
        LocalUser = 3,
        WritePlayer = 4,
        RobloxScript = 5,
        Roblox = 6,
        RobloxExecutor = 7,
        RobloxEngine = 8,
    };
}
