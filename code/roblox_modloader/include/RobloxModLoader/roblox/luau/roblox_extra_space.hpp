#pragma once
#include "RobloxModLoader/roblox/security/script_permissions.hpp"
#include "RobloxModLoader/util/layout_assert.hpp"

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
    };

    class ThreadIdentityContext {
    public:
        ExtendedIdentity identity;

    private:
        std::byte padding_0[0x8];

    public:
        lua_State *bound_state;

    private:
        std::byte padding_1[0x8];

    public:
        uint64_t capabilities;
        void *capability_deriver;

    private:
        RML_LAYOUT_GUARD_BEGIN()
            RML_ASSERT_LAYOUT_OFFSET(ThreadIdentityContext, identity, 0x00);
            RML_ASSERT_LAYOUT_OFFSET(ThreadIdentityContext, bound_state, 0x18);
            RML_ASSERT_LAYOUT_OFFSET(ThreadIdentityContext, capabilities, 0x28);
            RML_ASSERT_LAYOUT_OFFSET(ThreadIdentityContext, capability_deriver, 0x30);
        RML_LAYOUT_GUARD_END()
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

    private:
        // the mask keeps moving: 0x40 on 730, 0x90 on 732, 0x48 here. everything past it is still
        // the 730 layout and has not been re-checked against this build.
        std::byte padding_capabilities_gap[0x8];

    public:
        uint64_t capabilities;

    private:
        std::byte padding_capabilities[0x8];

    public:
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

    private:
        RML_LAYOUT_GUARD_BEGIN()
            RML_ASSERT_LAYOUT_SIZE(RobloxExtraSpace, 0x90);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, shared, 0x18);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, capabilities_validator, 0x28);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, context, 0x30);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, capabilities, 0x48);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, script, 0x58);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, actor, 0x70);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, is_actor_state, 0x88);
            RML_ASSERT_LAYOUT_OFFSET(RobloxExtraSpace, task_state, 0x89);
        RML_LAYOUT_GUARD_END()
    };
}
