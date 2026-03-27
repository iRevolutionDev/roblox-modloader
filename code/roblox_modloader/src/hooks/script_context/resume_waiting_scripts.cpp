#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"

void hooks::resume_waiting_scripts(uintptr_t *script_context, const int expiration_time) {
    return hooking::get_original<&hooks::resume_waiting_scripts>()(script_context, expiration_time);
}
