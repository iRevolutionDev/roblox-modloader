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
		g_main_thread = CreateThread(
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
					    CloseHandle(g_main_thread);
					    FreeLibraryAndExitThread(g_hinstance, 0);
					    return 1;
				    }

				    application.run();

				    application.shutdown();
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
		    0,
		    nullptr);
	}

	return TRUE;
}
