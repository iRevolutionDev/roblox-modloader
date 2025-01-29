#pragma once
#include <string>

#include "RobloxModLoader/common.hpp"

struct metadata {
    const std::string name{};
    const std::string version{};
    const std::string author{};
    const std::string description{};
};

class mod_base {
public:
    using start_type = mod_base*(*)();
    using uninstall_type = void(*)();

    std::string name{};
    std::string version{};
    std::string author{};
    std::string description{};
    uninstall_type uninstall_mod_func{};

    RML_API mod_base();

    RML_API virtual ~mod_base();

    RML_API virtual void on_load() = 0;
};
