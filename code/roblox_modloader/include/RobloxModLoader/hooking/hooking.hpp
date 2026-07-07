#pragma once

#include "RobloxModLoader/roblox/job_types.hpp"
#include "RobloxModLoader/rml_export.hpp"
#include "detour_hook.hpp"
#include "vtable_hook.hpp"

#include <MinHook.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct hooks;

class minhook_keepalive
{
public:
	minhook_keepalive()
	{
		MH_Initialize();
	}

	~minhook_keepalive()
	{
		MH_Uninitialize();
	}
};

class hooking
{
	friend hooks;

public:
	explicit hooking();

	~hooking();

	void enable();

	void disable();

	class RML_EXPORT detour_hook_helper
	{
		friend hooking;

		using ret_ptr_fn = std::function<void*()>;

		ret_ptr_fn m_on_hooking_available = nullptr;

		detour_hook* m_detour_hook;

		void enable_hook_if_hooking_is_already_running() const;

		template<auto detour_function>
		struct hook_to_detour_hook_helper
		{
			static inline detour_hook m_detour_hook;
		};

	public:
		template<auto detour_function>
		static void add(const std::string& name, void* target)
		{
			hook_to_detour_hook_helper<detour_function>::m_detour_hook.set_instance(name, target, detour_function);

			detour_hook_helper d{};
			d.m_detour_hook = &hook_to_detour_hook_helper<detour_function>::m_detour_hook;

			d.enable_hook_if_hooking_is_already_running();

			m_detour_hook_helpers.push_back(d);
		}

		template<auto detour_function>
		static void* add_lazy(const std::string& name, detour_hook_helper::ret_ptr_fn on_hooking_available)
		{
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
	static auto get_original()
	{
		return detour_hook_helper::hook_to_detour_hook_helper<detour_function>::m_detour_hook.get_original<decltype(detour_function)>();
	}

private:
	bool m_enabled{};
	minhook_keepalive m_minhook_keepalive;
	std::unordered_map<rml::JobKind, std::unique_ptr<vtable_hook> > m_jobs_hook;

	static inline std::vector<detour_hook_helper> m_detour_hook_helpers;
};

inline hooking* g_hooking{};
