#include "RobloxModLoader/common.hpp"
#include "mod_manager.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"
#include "RobloxModLoader/config/config.hpp"
#include "RobloxModLoader/config/config_helpers.hpp"
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
    const auto module_dir = get_module_directory();
    load_mods_from_directory(module_dir / "RobloxModLoader" / "mods");
    LOG_INFO("ModManager initialized.");
}

std::filesystem::path mod_manager::get_module_directory() {
    HMODULE hModule = nullptr;

    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&get_module_directory), &hModule)) {
        LOG_ERROR("Failed to get module handle: {}", GetLastError());
        return std::filesystem::current_path();
    }

    wchar_t module_path[MAX_PATH];
    if (GetModuleFileNameW(hModule, module_path, MAX_PATH) == 0) {
        LOG_ERROR("Failed to get module filename: {}", GetLastError());
        return std::filesystem::current_path();
    }

    std::filesystem::path module_dir = std::filesystem::path(module_path).parent_path();
    LOG_DEBUG("Module directory: {}", module_dir.string());
    return module_dir;
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
    for (const auto &mod_instance: mods) {
        if (mod_instance->uninstall_mod_func) {
            mod_instance->uninstall_mod_func();
        }
        mod_instance->on_unload();
    }
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

void mod_manager::set_event_manager(events::EventManager *manager) {
    event_manager = manager;

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

    for (const auto &mod_entry: std::filesystem::directory_iterator(directory_path)) {
        if (!mod_entry.is_directory()) {
            continue;
        }

        const auto mod_directory = mod_entry.path();
        const auto mod_name = mod_directory.filename().string();
        const auto native_directory = mod_directory / "native";

        LOG_INFO("Processing mod directory: {}", mod_name);

        if (!std::filesystem::exists(native_directory)) {
            LOG_WARN("No native directory found for mod: {}", mod_name);
            continue;
        }

        if (const auto config_path = mod_directory / "mod.toml"; std::filesystem::exists(config_path)) {
            LOG_INFO("Found mod.toml for mod: {}", mod_name);
            if (const auto config_result = rml::config::helpers::load_mod_config_from_file(mod_name, config_path); !
                config_result) {
                LOG_ERROR("Failed to load configuration for mod: {}", mod_name);
                continue;
            }

            if (!rml::config::is_mod_enabled(mod_name)) {
                LOG_INFO("Mod '{}' is disabled, skipping", mod_name);
                continue;
            }
        } else {
            LOG_WARN("No config.toml found for mod: {}, using default settings", mod_name);
        }

        for (const auto &dll_entry: std::filesystem::directory_iterator(native_directory)) {
            if (dll_entry.path().extension() != ".dll") {
                continue;
            }

            const auto &dll_path = dll_entry.path();
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

    load_mods();
}
