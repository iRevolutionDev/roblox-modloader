#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/core/entry_point.hpp"

RML_LOG_SCOPE("Bootstrap");

namespace rml
{
	void EntryPoint::attach(void* module_handle, const Driver driver) noexcept
	{
		static std::atomic_flag s_bootstrap_started;
		if (s_bootstrap_started.test_and_set(std::memory_order_acq_rel))
			return;

		Dl_info info{};
		if (dladdr(module_handle, &info))
			g_hinstance = info.dli_fbase;

		std::thread(driver).detach();
	}
}
