#pragma once

#define luau_limit_stack(L, max_size) { if (lua_gettop(L) > max_size) lua_settop(L, max_size); }

#define luau_check_stack(L, push_count) lua_rawcheckstack(L, push_count)

#define luau_prepare_gc_push(L, push_count) { luau_check_stack(L, push_count); luaC_threadbarrier(L); }

#define luau_validate_args(L, max_size) { if (lua_gettop(L) > max_size) luaL_errorL(L, "function expects only %d arguments, you provided %d.", max_size, lua_gettop(L)); }

#define luau_to_mutable_closure(L, index) const_cast<Closure*>(reinterpret_cast<const Closure*>(lua_topointer(L, index)))

#define luau_to_closure(L, index) reinterpret_cast<const Closure*>(lua_topointer(L, index))
