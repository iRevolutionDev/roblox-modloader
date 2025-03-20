#pragma once

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

class mod_manager {
public:
    mod_manager();

    ~mod_manager();

    void load_mods();

    void unload_mods();

    void load_mod(const std::shared_ptr<mod_base> &mod_instance);

    void register_mod(const std::shared_ptr<mod_base> &mod_instance);

    void unregister_mod(const std::shared_ptr<mod_base> &mod_instance);

    void load_mods_from_directory(const std::filesystem::path &directory_path);

    std::vector<std::shared_ptr<mod_base> > mods;
    std::mutex mods_mutex;
};

inline mod_manager *g_mod_manager{};
