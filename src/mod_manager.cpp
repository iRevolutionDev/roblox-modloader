#include "common.hpp"
#include "mod_manager.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include <filesystem>
#include <Windows.h>

mod_manager::mod_manager() {
    g_mod_manager = this;
    load_mods_from_directory(std::filesystem::current_path() / "RobloxModLoader" / "mods");
}

mod_manager::~mod_manager() {
    g_mod_manager = nullptr;
}

void mod_manager::load_mods() {
    std::lock_guard lock(mods_mutex);
    for (const auto &mod_instance: mods) {
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
    mod->on_load();
}

void mod_manager::register_mod(const std::shared_ptr<mod_base> &mod_instance) {
    std::lock_guard lock(mods_mutex);
    mods.push_back(mod_instance);
}

void mod_manager::unregister_mod(const std::shared_ptr<mod_base> &mod_instance) {
    std::lock_guard lock(mods_mutex);
    mods.erase(std::ranges::remove(mods, mod_instance).begin(), mods.end());
}

void mod_manager::load_mods_from_directory(const std::filesystem::path &directory_path) {
    if (!exists(directory_path)) {
        LOG_WARN("Directory does not exist: {}", directory_path.string());
        return;
    }

    for (const auto &entry: std::filesystem::directory_iterator(directory_path)) {
        if (entry.path().extension() == ".dll") {
            const auto &dll_path = entry.path();
            const auto dll_module = LoadLibraryExW(dll_path.c_str(), nullptr,
                                                   LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

            if (!dll_module) {
                LOG_WARN("Failed to load DLL: {}", dll_path.string());
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
            mod_instance->on_load();

            LOG_INFO("Loaded mod: {} ({} v{})", mod_instance->name, mod_instance->author, mod_instance->version);
        }
    }
}
