#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"

void hooks::resume_waiting_scripts(uintptr_t *script_context, const int expiration_time) {
    return hooking::get_original<&hooks::resume_waiting_scripts>()(script_context, expiration_time);
}
