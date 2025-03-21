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
