#pragma once
#include <string>
#include "events.hpp"
#include "RobloxModLoader/common.hpp"

struct metadata {
    const std::string name{};
    const std::string version{};
    const std::string author{};
    const std::string description{};
};

class RML_EXPORT mod_base {
public:
    using start_type = mod_base*(*)();
    using uninstall_type = void(*)();

    std::string name{};
    std::string version{};
    std::string author{};
    std::string description{};
    uninstall_type uninstall_mod_func{};

    mod_base();

    virtual ~mod_base();

    virtual void on_load() = 0;

    virtual void on_unload() = 0;

    virtual void on_script_manager_load() {
    }

    void set_event_manager(events::EventManager *manager);

protected:
    template<typename T>
    void register_event_handler(events::EventManager::EventHandler<T> handler) {
        if (event_manager) {
            event_manager->registerHandler<T>(handler);
        }
    }

private:
    events::EventManager *event_manager{nullptr};
};
