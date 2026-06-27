#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

ModBase::ModBase() {
    if (version.empty()) {
        version = "1.0.0";
    }
}

ModBase::~ModBase() {
}

void ModBase::set_event_manager(events::EventManager* manager) {
    event_manager = manager;
}
