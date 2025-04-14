#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"

void hooks::render_view(uintptr_t *scene_manager, uintptr_t *context, uintptr_t *mainFrameBuffer, uintptr_t *camera,
                        unsigned int viewWidth, unsigned int viewHeight) {
    hooking::get_original<&
        hooks::render_view>()(scene_manager, context, mainFrameBuffer, camera, viewWidth, viewHeight);
}
