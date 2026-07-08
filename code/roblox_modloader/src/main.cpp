#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/version.hpp"
#include "app/application.hpp"

RML_LOG_SCOPE("Bootstrap");

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD dwReason, LPVOID lp_reserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		static std::atomic_flag s_bootstrap_started;
		if (s_bootstrap_started.test_and_set(std::memory_order_acq_rel))
			return TRUE;

		g_hinstance = hModule;

		const HANDLE bootstrap_thread = CreateThread(
		    nullptr,
		    0,
		    [](PVOID) -> DWORD {
			    try
			    {
				    RML_INFO("Initializing Roblox Mod Loader {}...", rml::version::string());

				    rml::Application application;

				    if (const auto result = application.initialize(); !result)
				    {
					    std::cerr << "[RML] Fatal error during bootstrap: " << result.error().subsystem_name << ": " << result.error().message << std::endl;
					    application.shutdown();
				    }
				    else
				    {
					    application.run();
					    application.shutdown();
				    }
			    }
			    catch (const std::exception& e)
			    {
				    std::cerr << "[RML] Fatal error during bootstrap: " << e.what() << std::endl;
			    }
			    catch (...)
			    {
				    std::cerr << "[RML] Fatal error during bootstrap (unknown exception)" << std::endl;
			    }

			    CloseHandle(g_main_thread);
			    FreeLibraryAndExitThread(g_hinstance, 0);
		    },
		    nullptr,
		    CREATE_SUSPENDED,
		    nullptr);

		if (!bootstrap_thread)
		{
			std::cerr << "[RML] Failed to create the bootstrap thread" << std::endl;
			return TRUE;
		}

		g_main_thread = bootstrap_thread;
		ResumeThread(bootstrap_thread);
	}

	return TRUE;
}
