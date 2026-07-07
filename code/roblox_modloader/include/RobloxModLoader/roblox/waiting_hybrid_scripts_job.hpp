#pragma once
#include "data_model_job.hpp"
#include "script_context.hpp"

#include "RobloxModLoader/util/layout_assert.hpp"

namespace RBX::ScriptContextFacets {
    class WaitingHybridScriptsJob : public DataModelJob {
        char padding[0x1B0];

    public:
        ScriptContext *script_context;

    private:
        RML_LAYOUT_GUARD_BEGIN()
            RML_ASSERT_LAYOUT_SIZE(WaitingHybridScriptsJob, 0x200);
            RML_ASSERT_LAYOUT_OFFSET(WaitingHybridScriptsJob, padding, 0x48);
            RML_ASSERT_LAYOUT_OFFSET(WaitingHybridScriptsJob, script_context, 0x1F8);
        RML_LAYOUT_GUARD_END()
    };
}
