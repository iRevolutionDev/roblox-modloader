#pragma once

#include "type.hpp"

#pragma warning(disable : 4311)
#pragma warning(disable : 4312)

class PropertyDescriptor : public MemberDescriptor {
    unsigned is_public_: 1;
    unsigned is_editable_: 1;
    unsigned can_replicate_: 1;
    unsigned can_xml_read_: 1;
    unsigned can_xml_write_: 1;
    unsigned is_scriptable_: 1;
    unsigned always_clone_: 1;
    unsigned is_transient_: 1;

public:
    const Type &type;
    const bool is_enum;

    template<typename T = std::uint64_t>
    T getter() {
        const auto property_get_set = *reinterpret_cast<std::uint64_t *>(
            reinterpret_cast<std::uint64_t>(this) + (type.name == "Object" ? 0xA0 : 0xA8));
        return *reinterpret_cast<T *>(property_get_set + 0x8);
    }

    template<typename Return = std::uint64_t, typename This>
    Return getter(This *this_ptr) {
        return getter<Return(__fastcall*)(This *)>()(this_ptr);
    }

    template<typename T = std::uint64_t>
    T setter() {
        const auto property_get_set = *reinterpret_cast<std::uint64_t *>(
            reinterpret_cast<std::uint64_t>(this) + (type.name == "Object" ? 0xA8 : 0xA0));
        return *reinterpret_cast<T *>(property_get_set + 0x10);
    }

    template<typename Return = std::uint64_t, typename This, typename Arg>
    Return setter(This *this_ptr, Arg arg) {
        return setter<Return(__thiscall*)(This *, Arg)>()(this_ptr, arg);
    }
};
