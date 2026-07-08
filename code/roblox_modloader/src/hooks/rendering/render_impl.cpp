#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"
#include "RobloxModLoader/roblox/render_view.hpp"

void rml::Hooks::render_prepare(RenderView* this_ptr, uintptr_t metric, bool updateViewport)
{
	Hooking::get_original<&Hooks::render_prepare>()(this_ptr, metric, updateViewport);
}
