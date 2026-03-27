#pragma once

#include "property_descriptor.hpp"

class RefPropertyDescriptor : PropertyDescriptor {
    typedef PropertyDescriptor Super;

public:
    static bool is_ref_property_descriptor(const Type &type) {
        return type.Name == "Object";
    }

    static bool is_ref_property_descriptor(const PropertyDescriptor &descriptor) {
        return is_ref_property_descriptor(descriptor.type);
    }
};
