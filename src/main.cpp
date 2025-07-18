#include "RobloxModLoader/common.hpp"
#include "mod_manager.hpp"
#include "pointers.hpp"
#include "RobloxModLoader/hooking/hooking.hpp"
#include "RobloxModLoader/mod/events.hpp"
#include "RobloxModLoader/exception/exception_filter.hpp"
#include "RobloxModLoader/memory/rtti_scanner.hpp"
#include "RobloxModLoader/roblox/job_manager.hpp"
#include "RobloxModLoader/roblox/task_scheduler.hpp"
#include "RobloxModLoader/luau/script_manager.hpp"
#include "utils/directory_utils.hpp"

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);

        g_hinstance = hModule;
        g_main_thread = CreateThread(nullptr, 0, [](PVOID) -> DWORD {
            const auto config_path = directory_utils::get_module_directory() / "RobloxModLoader" / "config.toml";
            if (const auto config_result = rml::config::initialize(config_path, true); !config_result) {
                std::cerr << "Failed to initialize configuration system" << std::endl;
                return 1;
            }

            logger::init();

            LOG_INFO("Initializing Roblox Mod Loader...");

            exception_filter::exception_handler::initialize();
            LOG_INFO("Exception handler initialized.");

            const auto event_manager_instance = std::make_shared<events::EventManager>();
            LOG_INFO("Event Manager initialized.");

            const auto mod_manager_instance = std::make_shared<mod_manager>();
            LOG_INFO("Mod Manager created.");

            mod_manager_instance->initialize();
            LOG_INFO("Mods initialized.");

            mod_manager_instance->set_event_manager(event_manager_instance.get());
            LOG_INFO("Event Manager set in Mod Manager.");

            const auto rtti_manager_instance = std::make_shared<memory::rtti::rtti_manager>();
            LOG_INFO("RTTI Scanner initialized.");

            const auto pointers_instance = std::make_shared<pointers>();
            LOG_INFO("Pointers initialized.");

            const auto rbx_task_scheduler_instance = std::make_shared<RBX::TaskScheduler>();
            LOG_INFO("Roblox Task Scheduler initialized.");

            const auto job_manager_instance = std::make_shared<rml::jobs::JobManager>();
            LOG_INFO("Job Manager initialized.");

            const auto hooking_instance = std::make_shared<hooking>();
            LOG_INFO("Hooking initialized.");

            const auto script_manager = std::make_shared<rml::luau::ScriptManager>();
            LOG_INFO("Script Manager initialized.");

            const auto module_dir = directory_utils::get_module_directory();
            const auto mods_directory = module_dir / "RobloxModLoader" / "mods";
            script_manager->load_mod_scripts(mods_directory);
            LOG_INFO("Mod scripts loaded.");

            g_hooking->enable();
            LOG_INFO("Hooking enabled.");

            g_running = true;
            while (g_running) {
                std::this_thread::sleep_for(1s);
            }

            // Shutdown configuration system
            rml::config::shutdown();
            LOG_INFO("Configuration system shutdown.");

            exception_filter::exception_handler::shutdown();
            LOG_INFO("Exception handler shutdown.");

            CloseHandle(g_main_thread);
            FreeLibraryAndExitThread(g_hinstance, 0);
        }, nullptr, 0, nullptr);
    }

    return TRUE;
}
