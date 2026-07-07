#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"

void hooks::render_pass_3d(uintptr_t *_this, AdornRender *adorn) {
    hooking::get_original<&hooks::render_pass_3d>()(_this, adorn);
}
