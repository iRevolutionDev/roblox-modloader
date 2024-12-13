#include "common.hpp"

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD dwReason, LPVOID lpReserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);

		g_hinstance = hModule;
		g_main_thread = CreateThread(nullptr, 0, [](PVOID) -> DWORD {
			logger::init();

			LOG_INFO("Initializing Roblox Mod Loader...");


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
