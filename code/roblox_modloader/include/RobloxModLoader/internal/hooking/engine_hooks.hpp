#pragma once

#include "RobloxModLoader/roblox/adorn_render.hpp"
#include "RobloxModLoader/roblox/job.hpp"
#include "RobloxModLoader/roblox/render_view.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"

#include <cstdint>
#include <lua.h>

namespace RBX
{
	class TaskSchedulerJob;
}

struct hooks
{
	static void rbx_crash(const char* type, const char* message);
	static uint64_t* on_authentication(uint64_t* _this, uint64_t doc_panel_provider, uint64_t q_image_provider);
	static std::uintptr_t* build_summary(uintptr_t* _this, std::uintptr_t* out);
	static void render_pass_2d(uintptr_t* _this, AdornRender* adorn, uintptr_t* graphics_metric);
	static void render_pass_3d(uintptr_t* _this, AdornRender* adorn);
	static void render_prepare(RenderView* this_ptr, uintptr_t metric, bool updateViewport);
	static void render_perform(RenderView* this_ptr, double timeJobStart, uintptr_t* frame_buffer, uintptr_t a4);
	static void render_view(uintptr_t* scene_manager, uintptr_t* context, uintptr_t* mainFrameBuffer, uintptr_t* camera, uintptr_t* a5, unsigned int viewWidth, unsigned int viewHeight);
	static RBX::TaskScheduler::StepResult on_job_step(void** this_ptr, const RBX::Stats& time_metrics);
	static void on_job_destroy(void** this_ptr);
	static void resume_waiting_scripts(uintptr_t* script_context, int expiration_time);
	static void light_grid_update_perform(void* this_ptr, uintptr_t unk, void* unk2, uintptr_t unk3);
	static uintptr_t profile_log(uintptr_t token, uint64_t tick, uint64_t begin, uintptr_t* log);
	static lua_Status luau_load(lua_State* L, const char* chunkname, const char* data, size_t size, int env);
	static void* build_menu_bar_from_dom(void* out_menu_bar, void* dom, void* context);
	static void qt_action_activate(void* self, int event);
};
