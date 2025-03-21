#pragma once

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include "RobloxModLoader/mod/events.hpp"

class mod_manager {
public:
    mod_manager();

    ~mod_manager();

    void initialize();

    void load_mods();

    void unload_mods();

    void load_mod(const std::shared_ptr<mod_base> &mod_instance);

    void register_mod(const std::shared_ptr<mod_base> &mod_instance);

    void unregister_mod(const std::shared_ptr<mod_base> &mod_instance);

    void load_mods_from_directory(const std::filesystem::path &directory_path);

    void set_event_manager(events::EventManager *manager);

    std::vector<std::shared_ptr<mod_base> > mods;
    std::mutex mods_mutex;

private:
    events::EventManager *event_manager{nullptr};
};

inline mod_manager *g_mod_manager{};
