#pragma once

#include "descriptor.hpp"

class Type : public Descriptor {
public:
    const std::string &tag;
    const bool is_float;
    const bool is_number;
    const bool is_enum;
};
