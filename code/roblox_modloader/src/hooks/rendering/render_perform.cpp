#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/internal/hooking/engine_hooks.hpp"
#include "RobloxModLoader/roblox/render_view.hpp"

void rml::Hooks::render_perform(RenderView* this_ptr, double timeJobStart, uintptr_t* frame_buffer, uintptr_t a4)
{
	Hooking::get_original<&Hooks::render_perform>()(this_ptr, timeJobStart, frame_buffer, a4);
}
