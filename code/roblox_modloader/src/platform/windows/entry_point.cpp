#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/core/entry_point.hpp"

RML_LOG_SCOPE("Bootstrap");

namespace rml
{
	static EntryPoint::Driver s_driver = nullptr;

	void EntryPoint::attach(void* module_handle, const Driver driver) noexcept
	{
		const auto module = static_cast<HMODULE>(module_handle);
		DisableThreadLibraryCalls(module);

		static std::atomic_flag s_bootstrap_started;
		if (s_bootstrap_started.test_and_set(std::memory_order_acq_rel))
			return;

		g_hinstance = module;
		s_driver = driver;

		const HANDLE bootstrap_thread = CreateThread(
		    nullptr,
		    0,
		    [](LPVOID) -> DWORD {
			    s_driver();

			    CloseHandle(g_main_thread);
			    FreeLibraryAndExitThread(g_hinstance, 0);
		    },
		    nullptr,
		    CREATE_SUSPENDED,
		    nullptr);

		if (!bootstrap_thread)
		{
			std::cerr << "[RML] Failed to create the bootstrap thread" << std::endl;
			return;
		}

		g_main_thread = bootstrap_thread;
		ResumeThread(bootstrap_thread);
	}
}
