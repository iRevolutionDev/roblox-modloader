#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"

void rml::Hooks::render_view(uintptr_t* scene_manager, uintptr_t* context, uintptr_t* mainFrameBuffer, uintptr_t* camera, uintptr_t* a5, unsigned int viewWidth, unsigned int viewHeight)
{
	Hooking::get_original<&Hooks::render_view>()(scene_manager, context, mainFrameBuffer, camera, a5, viewWidth, viewHeight);
}
