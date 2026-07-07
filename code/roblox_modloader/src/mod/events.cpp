#include "RobloxModLoader/mod/events.hpp"
#include "RobloxModLoader/internal/common.hpp"

namespace rml::events {
    static EventManager *s_active_event_manager{};

    EventManager::EventManager() {
        LOG_INFO("Event Manager initialized.");
        s_active_event_manager = this;
    }

    EventManager::~EventManager() {
        s_active_event_manager = nullptr;
    }

    EventManager &event_manager() {
        return *s_active_event_manager;
    }
}
