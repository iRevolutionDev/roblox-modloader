#pragma once

#define luau_to_closure(L, index) reinterpret_cast<const Closure*>(lua_topointer(L, index))
