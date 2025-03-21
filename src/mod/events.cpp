#include "RobloxModLoader/mod/events.hpp"
#include "RobloxModLoader/common.hpp"

namespace events {
    EventManager::EventManager() {
        LOG_INFO("Event Manager initialized.");
        g_event_manager = this;
    }

    EventManager::~EventManager() {
        g_event_manager = nullptr;
    }
}
