#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"
#include "RobloxModLoader/roblox/render_view.hpp"
#include <vector>
#include <cmath>

void hooks::render_perform(RenderView *this_ptr, uintptr_t mainFramebuffer, double timeJobStart) {
    hooking::get_original<&hooks::render_perform>()(this_ptr, mainFramebuffer, timeJobStart);
void hooks::render_perform(RenderView *this_ptr, double timeJobStart, uintptr_t *frame_buffer, uintptr_t a4) {
    hooking::get_original<&hooks::render_perform>()(this_ptr, timeJobStart, frame_buffer, a4);
}
