#pragma once
#include "member.hpp"

class FunctionDescriptor : public MemberDescriptor {
public:
    template<typename T = uintptr_t>
    T get_bound_function() {
        return *reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(this) + 0x88);
    }

    template<typename Return>
    Return invoke() {
        auto func = get_bound_function<Return>();
        return func;
    }

    template<typename Return = std::uint64_t, typename This>
    Return invoke(This this_ptr) {
        return invoke<Return(__fastcall*)(This)>()(this_ptr);
    }

    template<typename Return = std::uint64_t, typename This, typename Arg1>
    Return invoke(This *this_ptr, Arg1 arg1) {
        return invoke<Return(__fastcall*)(This *, Arg1)>()(this_ptr, arg1);
    }

    template<typename Return = std::uint64_t, typename This, typename Arg1, typename Arg2>
    Return invoke(This *this_ptr, Arg1 arg1, Arg2 arg2) {
        return invoke<Return(__fastcall*)(This *, Arg1, Arg2)>()(this_ptr, arg1, arg2);
    }

    template<typename Return = std::uint64_t, typename This, typename Arg1, typename Arg2, typename Arg3>
    Return invoke(This *this_ptr, Arg1 arg1, Arg2 arg2, Arg3 arg3) {
        return invoke<Return(__fastcall*)(This *, Arg1, Arg2, Arg3)>()(this_ptr, arg1, arg2, arg3);
    }

    template<typename Return = std::uint64_t, typename This, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    Return invoke(This *this_ptr, Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
        return invoke<Return(__fastcall*)(This *, Arg1, Arg2, Arg3, Arg4)>()(this_ptr, arg1, arg2, arg3, arg4);
    }
};
