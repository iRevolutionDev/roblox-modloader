#pragma once

#include <MinHook.h>
#include "detour_hook.hpp"
#include "vmt_hook.hpp"
#include "vtable_hook.hpp"
#include "call_hook.hpp"
#include "RobloxModLoader/mod/events.hpp"
#include "RobloxModLoader/roblox/adorn_render.hpp"
#include "RobloxModLoader/roblox/job.hpp"
#include "RobloxModLoader/roblox/render_view.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"

namespace RBX {
	class TaskSchedulerJob;
}

struct hooks {
	static void rbx_crash(const char *type, const char *message);

	static uint64_t *on_authentication(uint64_t *_this, uint64_t doc_panel_provider, uint64_t q_image_provider);

	static std::uintptr_t *build_summary(uintptr_t *_this, std::uintptr_t *out);

	static void render_pass_2d(uintptr_t *_this, AdornRender *adorn, uintptr_t *graphics_metric);

	static void render_pass_3d(uintptr_t *_this, AdornRender *adorn);

	static void render_prepare(RenderView *this_ptr, uintptr_t metric, bool updateViewport);

	static void render_perform(RenderView *this_ptr, double timeJobStart, uintptr_t *frame_buffer, uintptr_t a4);

	static void render_view(uintptr_t *scene_manager, uintptr_t *context, uintptr_t *mainFrameBuffer, uintptr_t *camera,
	                        uintptr_t *a5, unsigned int viewWidth, unsigned int viewHeight);

	static RBX::TaskScheduler::StepResult on_job_step(void **this_ptr, const RBX::Stats &time_metrics);

	static void on_job_destroy(void **this_ptr);

	static void resume_waiting_scripts(uintptr_t *script_context, int expiration_time);

	static void light_grid_update_perform(void *this_ptr, uintptr_t unk, void *unk2, uintptr_t unk3);

	static uintptr_t profile_log(uintptr_t token, uint64_t tick, uint64_t begin, uintptr_t *log);

	static lua_Status *luau_load(lua_State *L, const char *chunkname, const char *data, size_t size,
	                             int env);
};

class minhook_keepalive {
public:
	minhook_keepalive() {
		MH_Initialize();
	}

	~minhook_keepalive() {
		MH_Uninitialize();
	}
};

class hooking {
	friend hooks;

public:
	explicit hooking();

	~hooking();

	void enable();

	void disable();

	class detour_hook_helper {
		friend hooking;

		using ret_ptr_fn = std::function<void*()>;

		ret_ptr_fn m_on_hooking_available = nullptr;

		detour_hook *m_detour_hook;

		void enable_hook_if_hooking_is_already_running();

		template<auto detour_function>
		struct hook_to_detour_hook_helper {
			static inline detour_hook m_detour_hook;
		};

	public:
		template<auto detour_function>
		static void add(const std::string &name, void *target) {
			hook_to_detour_hook_helper<detour_function>::m_detour_hook.set_instance(name, target, detour_function);

			detour_hook_helper d{};
			d.m_detour_hook = &hook_to_detour_hook_helper<detour_function>::m_detour_hook;

			d.enable_hook_if_hooking_is_already_running();

			m_detour_hook_helpers.push_back(d);
		}

		template<auto detour_function>
		static void *add_lazy(const std::string &name, detour_hook_helper::ret_ptr_fn on_hooking_available) {
			hook_to_detour_hook_helper<detour_function>::m_detour_hook.set_instance(name, detour_function);

			detour_hook_helper d{};
			d.m_detour_hook = &hook_to_detour_hook_helper<detour_function>::m_detour_hook;
			d.m_on_hooking_available = on_hooking_available;

			d.enable_hook_if_hooking_is_already_running();

			m_detour_hook_helpers.push_back(d);

			return nullptr;
		}

		~detour_hook_helper();
	};

	template<auto detour_function>
	static auto get_original() {
		return detour_hook_helper::hook_to_detour_hook_helper<detour_function>::m_detour_hook.get_original<decltype(
			detour_function)>();
	}

private:
	bool m_enabled{};
	minhook_keepalive m_minhook_keepalive;
	std::unordered_map<rml::JobKind, std::unique_ptr<vtable_hook> > m_jobs_hook;

	static inline std::vector<detour_hook_helper> m_detour_hook_helpers;
};

inline hooking *g_hooking{};
