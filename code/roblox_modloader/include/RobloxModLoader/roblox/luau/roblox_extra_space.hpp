#pragma once
#include "RobloxModLoader/roblox/security/script_permissions.hpp"

namespace RBX {
    class ScriptContext;
}

namespace RBX {
    class Actor;
    class Script;
}

namespace RBX::Luau {
    typedef int64_t (*CapabilityValidator)(int64_t capability, lua_State *L);

    struct ExtendedIdentity {
        Security::Permissions identity;
        uint64_t asset_id;
        void *script_context;
    };


    enum TaskState : std::int8_t {
        None = 0,
        Deferred = 1,
        Delayed = 2,
        Waiting = 3,
    };

    class RobloxExtraSpace {
        struct Shared {
            int32_t thread_count;
            ScriptContext *context;
            uintptr_t *weak_ref;
            uintptr_t *intrusive_hook_all_threads;
        };

        std::byte padding_0[0x18];

    public:
        Shared *shared;

    private:
        std::byte padding_1[0x8]; // jump the trash boost shared_ptr padding.

    public:
        CapabilityValidator *capabilities_validator;
        ExtendedIdentity context;
        uint64_t capabilities;
        Script *script;

    private:
        std::byte padding_2[0x10];

    public:
        Actor *actor;

    private:
        std::byte padding_3[0x10];

    public:
        bool is_actor_state;
        TaskState task_state;
    };
}
