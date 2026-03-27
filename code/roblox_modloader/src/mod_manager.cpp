#include "mod_manager.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/config/config.hpp"
#include "RobloxModLoader/config/config_helpers.hpp"
#include "RobloxModLoader/dotnet/dotnet_host.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

#include <Windows.h>
#include <filesystem>

mod_manager::mod_manager()
{
	g_mod_manager = this;
}

mod_manager::~mod_manager()
{
	g_mod_manager = nullptr;
}

void mod_manager::initialize()
{
	LOG_INFO("Initializing ModManager...");
	const auto module_dir = get_module_directory();
	load_mods_from_directory(module_dir / "RobloxModLoader" / "mods");
	LOG_INFO("ModManager initialized.");
}

std::filesystem::path mod_manager::get_module_directory()
{
	HMODULE hModule = nullptr;

	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&get_module_directory), &hModule))
	{
		LOG_ERROR("Failed to get module handle: {}", GetLastError());
		return std::filesystem::current_path();
	}

	wchar_t module_path[MAX_PATH];
	if (GetModuleFileNameW(hModule, module_path, MAX_PATH) == 0)
	{
		LOG_ERROR("Failed to get module filename: {}", GetLastError());
		return std::filesystem::current_path();
	}

	std::filesystem::path module_dir = std::filesystem::path(module_path).parent_path();
	LOG_DEBUG("Module directory: {}", module_dir.string());
	return module_dir;
}

void mod_manager::load_mods()
{
	std::lock_guard lock(mods_mutex);
	for (const auto& mod_instance : mods)
	{
		if (event_manager)
		{
			mod_instance->set_event_manager(event_manager);
		}
		mod_instance->on_load();
	}
}

void mod_manager::unload_mods()
{
	for (const auto& managed_mod : m_managed_mods)
	{
		if (m_managed_bridge.initialized && m_managed_bridge.unload_mod)
		{
			const auto assembly_utf8 = path_to_utf8(managed_mod.assembly_path);
			m_managed_bridge.unload_mod(assembly_utf8.c_str());
		}
		else if (managed_mod.shutdown)
		{
			LOG_INFO("Shutting down managed mod: {}", managed_mod.mod_name);
			managed_mod.shutdown();
		}
	}
	m_managed_mods.clear();

	m_managed_bridge = {};

	m_dotnet_host.shutdown();

	std::lock_guard lock(mods_mutex);
	for (const auto& mod_instance : mods)
	{
		if (mod_instance->uninstall_mod_func)
		{
			mod_instance->uninstall_mod_func();
		}
		mod_instance->on_unload();
	}
	mods.clear();
}

void mod_manager::load_mod(const std::shared_ptr<mod_base>& mod)
{
	std::lock_guard lock(mods_mutex);
	mods.push_back(mod);
	if (event_manager)
	{
		mod->set_event_manager(event_manager);
	}
	mod->on_load();
}

void mod_manager::register_mod(const std::shared_ptr<mod_base>& mod_instance)
{
	std::lock_guard lock(mods_mutex);
	mods.push_back(mod_instance);
	if (event_manager)
	{
		mod_instance->set_event_manager(event_manager);
	}
	LOG_INFO("Registered mod: {} ({} v{})", mod_instance->name, mod_instance->author, mod_instance->version);
}

void mod_manager::unregister_mod(const std::shared_ptr<mod_base>& mod_instance)
{
	std::lock_guard lock(mods_mutex);
	mods.erase(std::ranges::remove(mods, mod_instance).begin(), mods.end());
}

void mod_manager::set_event_manager(events::EventManager* manager)
{
	event_manager = manager;

	std::lock_guard lock(mods_mutex);
	for (const auto& mod : mods)
	{
		mod->set_event_manager(manager);
	}
}

void mod_manager::load_mods_from_directory(const std::filesystem::path& directory_path)
{
	LOG_INFO("Loading mods from: {}", directory_path.string());

	if (!exists(directory_path))
	{
		LOG_WARN("Directory does not exist: {}", directory_path.string());
		create_directories(directory_path);
		LOG_INFO("Created mods directory: {}", directory_path.string());
		return;
	}

	for (const auto& mod_entry : std::filesystem::directory_iterator(directory_path))
	{
		if (!mod_entry.is_directory())
		{
			continue;
		}

		const auto mod_directory     = mod_entry.path();
		const auto mod_name          = mod_directory.filename().string();
		const auto native_directory  = mod_directory / "native";
		const auto managed_directory = mod_directory / "managed";

		LOG_INFO("Processing mod directory: {}", mod_name);

		const bool has_native  = std::filesystem::exists(native_directory);
		const bool has_managed = std::filesystem::exists(managed_directory);
		if (!has_native && !has_managed)
		{
			LOG_WARN("No native or managed directory found for mod: {}", mod_name);
			continue;
		}

		if (const auto config_path = mod_directory / "mod.toml"; std::filesystem::exists(config_path))
		{
			LOG_INFO("Found mod.toml for mod: {}", mod_name);
			if (const auto config_result = rml::config::helpers::load_mod_config_from_file(mod_name, config_path); !config_result)
			{
				LOG_ERROR("Failed to load configuration for mod: {}", mod_name);
				continue;
			}

			if (!rml::config::is_mod_enabled(mod_name))
			{
				LOG_INFO("Mod '{}' is disabled, skipping", mod_name);
				continue;
			}
		}
		else
		{
			LOG_WARN("No config.toml found for mod: {}, using default settings", mod_name);
		}

		if (has_native)
		{
			for (const auto& dll_entry : std::filesystem::directory_iterator(native_directory))
			{
				if (dll_entry.path().extension() != ".dll")
				{
					continue;
				}

				const auto& dll_path = dll_entry.path();
				LOG_INFO("Found mod DLL: {}", dll_path.string());

				const auto dll_module = LoadLibraryExW(dll_path.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

				if (!dll_module)
				{
					const auto error = GetLastError();
					LOG_WARN("Failed to load DLL: {} (Error: {})", dll_path.string(), error);
					continue;
				}

				const auto start_mod_func = reinterpret_cast<mod_base::start_type>(GetProcAddress(dll_module, "start_mod"));
				const auto uninstall_mod_func = reinterpret_cast<mod_base::uninstall_type>(GetProcAddress(dll_module, "uninstall_mod"));

				if (!start_mod_func || !uninstall_mod_func)
				{
					LOG_WARN("Failed to find start_mod or uninstall_mod functions in DLL: {}", dll_path.string());
					FreeLibrary(dll_module);
					continue;
				}

				const auto mod_instance          = start_mod_func();
				mod_instance->uninstall_mod_func = uninstall_mod_func;
				register_mod(std::shared_ptr<mod_base>(mod_instance));
			}
		}

		if (has_managed)
		{
			load_managed_mods_from_directory(mod_name, mod_directory, managed_directory);
		}
	}

	load_mods();
}

std::string mod_manager::path_to_utf8(const std::filesystem::path& path)
{
	return path.generic_string();
}

std::filesystem::path mod_manager::get_global_runtime_config_path()
{
	const auto module_dir = get_module_directory();

	const auto preferred = module_dir / "RobloxModLoader" / "runtime" / "RobloxModLoader.runtimeconfig.json";
	if (std::filesystem::exists(preferred))
	{
		return preferred;
	}

	const auto legacy = module_dir / "RobloxModLoader.runtimeconfig.json";
	if (std::filesystem::exists(legacy))
	{
		return legacy;
	}

	return {};
}

std::filesystem::path mod_manager::get_managed_bridge_assembly_path()
{
	const auto module_dir = get_module_directory();

	const auto preferred = module_dir / "RobloxModLoader" / "runtime" / "managed" / "RobloxModLoader.Managed.dll";
	if (std::filesystem::exists(preferred))
	{
		return preferred;
	}

	const auto fallback = module_dir / "RobloxModLoader" / "managed" / "RobloxModLoader.Managed.dll";
	if (std::filesystem::exists(fallback))
	{
		return fallback;
	}

	return {};
}

bool mod_manager::ensure_dotnet_host_initialized(const std::filesystem::path& runtime_config_path) const
{
	if (m_dotnet_host.is_initialized())
	{
		return true;
	}

	std::string error_message;
	if (!m_dotnet_host.initialize(runtime_config_path, &error_message))
	{
		LOG_ERROR("Failed to initialize dotnet host: {}", error_message);
		return false;
	}

	LOG_INFO("DotNet host initialized with runtime config: {}", runtime_config_path.string());
	return true;
}

bool mod_manager::ensure_managed_bridge_initialized()
{
	if (m_managed_bridge.initialized)
	{
		return true;
	}

	const auto runtime_config = get_global_runtime_config_path();
	if (runtime_config.empty())
	{
		LOG_WARN("Managed bridge init skipped: global runtimeconfig not found.");
		return false;
	}

	if (!ensure_dotnet_host_initialized(runtime_config))
	{
		return false;
	}

	m_managed_bridge.assembly_path = get_managed_bridge_assembly_path();
	if (m_managed_bridge.assembly_path.empty())
	{
		LOG_WARN("Managed bridge init skipped: RobloxModLoader.Managed.dll not found.");
		return false;
	}

	void* initialize_raw = nullptr;
	std::string error;
	if (!m_dotnet_host.load_assembly_entrypoint(m_managed_bridge.assembly_path, "RobloxModLoader.Managed.Host.ManagedRuntimeHost, RobloxModLoader.Managed", "rml_initialize", rml::dotnet::unmanaged_callers_only_method, &initialize_raw, &error) || !initialize_raw)
	{
		LOG_WARN("Failed to resolve managed bridge initialize entrypoint: {}", error);
		return false;
	}

	void* load_mod_raw = nullptr;
	if (!m_dotnet_host.load_assembly_entrypoint(m_managed_bridge.assembly_path, "RobloxModLoader.Managed.Host.ManagedRuntimeHost, RobloxModLoader.Managed", "rml_load_mod", rml::dotnet::unmanaged_callers_only_method, &load_mod_raw, &error) || !load_mod_raw)
	{
		LOG_WARN("Failed to resolve managed bridge load entrypoint: {}", error);
		return false;
	}

	void* unload_mod_raw = nullptr;
	if (!m_dotnet_host.load_assembly_entrypoint(m_managed_bridge.assembly_path, "RobloxModLoader.Managed.Host.ManagedRuntimeHost, RobloxModLoader.Managed", "rml_unload_mod", rml::dotnet::unmanaged_callers_only_method, &unload_mod_raw, &error) || !unload_mod_raw)
	{
		LOG_WARN("Failed to resolve managed bridge unload entrypoint: {}", error);
		return false;
	}

	m_managed_bridge.initialize = reinterpret_cast<managed_bridge_initialize_fn>(initialize_raw);
	m_managed_bridge.load_mod   = reinterpret_cast<managed_bridge_load_mod_fn>(load_mod_raw);
	m_managed_bridge.unload_mod = reinterpret_cast<managed_bridge_unload_mod_fn>(unload_mod_raw);

	const auto module_dir     = get_module_directory();
	const auto mods_root      = module_dir / "RobloxModLoader" / "mods";
	const auto generated_root = module_dir / "RobloxModLoader" / "generated";
	std::error_code ec;
	std::filesystem::create_directories(generated_root, ec);

	const auto mods_root_utf8      = path_to_utf8(mods_root);
	const auto generated_root_utf8 = path_to_utf8(generated_root);
	if (const int init_result = m_managed_bridge.initialize(mods_root_utf8.c_str(), generated_root_utf8.c_str()); init_result != 0)
	{
		LOG_WARN("Managed bridge initialize returned {}", init_result);
		return false;
	}

	m_managed_bridge.initialized = true;
	LOG_INFO("Managed runtime bridge initialized: {}", m_managed_bridge.assembly_path.string());
	return true;
}

bool mod_manager::try_load_managed_mod_via_bridge(const std::string& mod_name, const std::filesystem::path& mod_directory, const std::filesystem::path& assembly_path)
{
	if (!ensure_managed_bridge_initialized())
	{
		return false;
	}

	const auto assembly_utf8 = path_to_utf8(assembly_path);
	if (const int load_result = m_managed_bridge.load_mod(assembly_utf8.c_str()); load_result != 0)
	{
		LOG_WARN("Managed bridge failed to load mod '{}' from '{}': {}", mod_name, assembly_path.string(), load_result);
		return false;
	}

	m_managed_mods.push_back({
	    .mod_name      = mod_name,
	    .assembly_path = assembly_path,
	    .initialize    = nullptr,
	    .shutdown      = nullptr,
	});

	LOG_INFO("Managed mod loaded via centralized bridge: {}", assembly_path.string());
	return true;
}

void mod_manager::load_managed_mod(const std::string& mod_name, const std::filesystem::path& mod_directory, const std::filesystem::path& managed_directory, const std::filesystem::path& assembly_path)
{
	if (!try_load_managed_mod_via_bridge(mod_name, mod_directory, assembly_path))
	{
		LOG_WARN("Managed mod '{}' skipped because centralized managed bridge is unavailable: {}",
		    mod_name,
		    assembly_path.string());
	}
}

void mod_manager::load_managed_mods_from_directory(const std::string& mod_name, const std::filesystem::path& mod_directory, const std::filesystem::path& managed_directory)
{
	LOG_INFO("Scanning managed mods in: {}", managed_directory.string());

	for (const auto& assembly_entry : std::filesystem::directory_iterator(managed_directory))
	{
		if (!assembly_entry.is_regular_file())
		{
			continue;
		}

		if (assembly_entry.path().extension() != ".dll")
		{
			continue;
		}

		const auto filename = assembly_entry.path().filename().string();
		if (filename.contains(".resources."))
		{
			continue;
		}

		if (filename == "RobloxModLoader.Managed.dll")
		{
			continue;
		}

		load_managed_mod(mod_name, mod_directory, managed_directory, assembly_entry.path());
	}
}
