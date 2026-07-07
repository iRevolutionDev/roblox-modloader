#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"

void rml::Hooks::render_pass_2d(uintptr_t* _this, AdornRender* adorn, uintptr_t* graphics_metric)
{
	Hooking::get_original<&Hooks::render_pass_2d>()(_this, adorn, graphics_metric);
}
