#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/mod/events.hpp"

uint64_t *hooks::on_authentication(uint64_t *_this, uint64_t doc_panel_provider, uint64_t q_image_provider) {
    rml::events::AuthenticationEvent event(_this, doc_panel_provider, q_image_provider);
    rml::events::event_manager().emit(event);

    if (event.cancelled) {
        return nullptr;
    }

    return hooking::get_original<&hooks::on_authentication>()(_this, doc_panel_provider, q_image_provider);
}
