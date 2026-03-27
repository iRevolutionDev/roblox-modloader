#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"
#include "RobloxModLoader/roblox/render_view.hpp"

void hooks::render_prepare(RenderView *this_ptr, uintptr_t metric, bool updateViewport) {
    hooking::get_original<&hooks::render_prepare>()(this_ptr, metric, updateViewport);
}
