#pragma once
#include "data_model_job.hpp"
#include "script_context.hpp"

namespace RBX::ScriptContextFacets {
    class WaitingHybridScriptsJob : DataModelJob {
        char padding[0x1B0];

    public:
        std::shared_ptr<ScriptContext> script_context;
    };
}
