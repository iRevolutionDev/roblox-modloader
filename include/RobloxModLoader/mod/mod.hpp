#pragma once
#include <string>

struct metadata {
    const std::string name{};
    const std::string version{};
    const std::string author{};
    const std::string description{};
};

class mod {
public:
    using start_type = mod*(*)();
    using uninstall_type = void(*)();

    std::string name{};
    std::string version{};
    std::string author{};
    std::string description{};
    uninstall_type uninstall_mod_func{};

    mod();
    virtual ~mod();

    virtual void on_load() = 0;
};
