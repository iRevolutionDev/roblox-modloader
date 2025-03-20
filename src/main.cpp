#include "RobloxModLoader/common.hpp"
#include "mod_manager.hpp"

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);

		g_hinstance = hModule;
		g_main_thread = CreateThread(nullptr, 0, [](PVOID) -> DWORD {
			logger::init();

			LOG_INFO("Initializing Roblox Mod Loader...");

			const auto mod_manager_instance = std::make_shared<mod_manager>();

			g_running = true;
			while (g_running) {
				std::this_thread::sleep_for(1s);
			}

			CloseHandle(g_main_thread);
			FreeLibraryAndExitThread(g_hinstance, 0);
		}, nullptr, 0, nullptr);
	}

	return TRUE;
}
