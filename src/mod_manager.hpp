#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <filesystem>
#include "RobloxModLoader/mod/mod.hpp"

class mod_manager {
public:
    mod_manager();

    ~mod_manager();

    void load_mods();

    void unload_mods();

    void load_mod(const std::shared_ptr<mod> &mod_instance);

    void register_mod(const std::shared_ptr<mod> &mod_instance);

    void unregister_mod(const std::shared_ptr<mod> &mod_instance);

    void load_mods_from_directory(const std::filesystem::path &directory_path);

    std::vector<std::shared_ptr<mod> > mods;
    std::mutex mods_mutex;
};

inline mod_manager *g_mod_manager{};
