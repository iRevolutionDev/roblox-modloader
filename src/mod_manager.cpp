#include "RobloxModLoader/common.hpp"
#include "mod_manager.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include <filesystem>
#include <Windows.h>

mod_manager::mod_manager() {
    g_mod_manager = this;
}

mod_manager::~mod_manager() {
    g_mod_manager = nullptr;
}

void mod_manager::initialize() {
    LOG_INFO("Initializing ModManager...");
    load_mods_from_directory(std::filesystem::current_path() / "RobloxModLoader" / "mods");
    LOG_INFO("ModManager initialized.");
}

void mod_manager::load_mods() {
    std::lock_guard lock(mods_mutex);
    for (const auto &mod_instance: mods) {
        if (event_manager) {
            mod_instance->set_event_manager(event_manager);
        }
        mod_instance->on_load();
    }
}

void mod_manager::unload_mods() {
    std::lock_guard lock(mods_mutex);
    mods.clear();
}

void mod_manager::load_mod(const std::shared_ptr<mod_base> &mod) {
    std::lock_guard lock(mods_mutex);
    mods.push_back(mod);
    if (event_manager) {
        mod->set_event_manager(event_manager);
    }
    mod->on_load();
}

void mod_manager::register_mod(const std::shared_ptr<mod_base> &mod_instance) {
    std::lock_guard lock(mods_mutex);
    mods.push_back(mod_instance);
    if (event_manager) {
        mod_instance->set_event_manager(event_manager);
    }
    LOG_INFO("Registered mod: {} ({} v{})", mod_instance->name, mod_instance->author, mod_instance->version);
}

void mod_manager::unregister_mod(const std::shared_ptr<mod_base> &mod_instance) {
    std::lock_guard lock(mods_mutex);
    mods.erase(std::ranges::remove(mods, mod_instance).begin(), mods.end());
}

void mod_manager::set_event_manager(events::EventManager* manager) {
    event_manager = manager;
    // Atualiza os mods existentes com o novo manager
    std::lock_guard lock(mods_mutex);
    for (const auto &mod: mods) {
        mod->set_event_manager(manager);
    }
}

void mod_manager::load_mods_from_directory(const std::filesystem::path &directory_path) {
    LOG_INFO("Loading mods from: {}", directory_path.string());
    
    if (!exists(directory_path)) {
        LOG_WARN("Directory does not exist: {}", directory_path.string());
        create_directories(directory_path);
        LOG_INFO("Created mods directory: {}", directory_path.string());
        return;
    }

    for (const auto &entry: std::filesystem::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".dll") {
            const auto &dll_path = entry.path();
            LOG_INFO("Found mod DLL: {}", dll_path.string());
            
            const auto dll_module = LoadLibraryExW(dll_path.c_str(), nullptr,
                                               LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

            if (!dll_module) {
                const auto error = GetLastError();
                LOG_WARN("Failed to load DLL: {} (Error: {})", dll_path.string(), error);
                continue;
            }

            const auto start_mod_func = reinterpret_cast<mod_base::start_type>(GetProcAddress(dll_module, "start_mod"));
            const auto uninstall_mod_func = reinterpret_cast<mod_base::uninstall_type>(GetProcAddress(
                dll_module, "uninstall_mod"));

            if (!start_mod_func || !uninstall_mod_func) {
                LOG_WARN("Failed to find start_mod or uninstall_mod functions in DLL: {}", dll_path.string());
                FreeLibrary(dll_module);
                continue;
            }

            const auto mod_instance = start_mod_func();
            mod_instance->uninstall_mod_func = uninstall_mod_func;
            register_mod(std::shared_ptr<mod_base>(mod_instance));
        }
    }

    // Agora que todos os mods estão registrados, inicializa-os
    load_mods();
}
