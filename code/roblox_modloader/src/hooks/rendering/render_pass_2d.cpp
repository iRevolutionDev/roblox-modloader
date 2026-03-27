#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"

void hooks::render_pass_2d(uintptr_t *_this, AdornRender *adorn, uintptr_t *graphics_metric) {
    hooking::get_original<&hooks::render_pass_2d>()(_this, adorn, graphics_metric);
}
