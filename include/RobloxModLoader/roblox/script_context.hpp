#pragma once
#include "instance.hpp"

namespace RBX {
    class ScriptContext : public Instance {
    public:
        lua_State *get_global_state();
    };
}
