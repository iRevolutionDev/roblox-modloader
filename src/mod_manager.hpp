#pragma once

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/dotnet/dotnet_host.hpp"
#include "RobloxModLoader/mod/events.hpp"
#include "RobloxModLoader/mod/mod_base.hpp"

class mod_manager
{
public:
	using managed_mod_initialize_fn = int (*)(const char* mod_root_utf8, const char* generated_output_dir_utf8);
	using managed_mod_shutdown_fn   = void (*)();

	struct managed_mod_state
	{
		std::string mod_name;
		std::filesystem::path assembly_path;
		managed_mod_initialize_fn initialize{nullptr};
		managed_mod_shutdown_fn shutdown{nullptr};
	};

	using managed_bridge_initialize_fn = int (*)(const char* mods_root_utf8, const char* generated_root_utf8);
	using managed_bridge_load_mod_fn   = int (*)(const char* assembly_path_utf8);
	using managed_bridge_unload_mod_fn = int (*)(const char* assembly_path_utf8);

	struct managed_bridge_state
	{
		std::filesystem::path assembly_path;
		managed_bridge_initialize_fn initialize{nullptr};
		managed_bridge_load_mod_fn load_mod{nullptr};
		managed_bridge_unload_mod_fn unload_mod{nullptr};
		bool initialized{false};
	};

	mod_manager();

	~mod_manager();

	void initialize();

	void load_mods();
	void unload_mods();

	void load_mod(const std::shared_ptr<mod_base>& mod_instance);
	void register_mod(const std::shared_ptr<mod_base>& mod_instance);
	void unregister_mod(const std::shared_ptr<mod_base>& mod_instance);

	void load_mods_from_directory(const std::filesystem::path& directory_path);
	void set_event_manager(events::EventManager* manager);

	std::vector<std::shared_ptr<mod_base>> mods;
	std::mutex mods_mutex;

private:
	static std::filesystem::path get_module_directory();
	static std::string path_to_utf8(const std::filesystem::path& path);
	static std::filesystem::path get_global_runtime_config_path();
	static std::filesystem::path get_managed_bridge_assembly_path();
	bool ensure_dotnet_host_initialized(const std::filesystem::path& runtime_config_path) const;
	bool ensure_managed_bridge_initialized();
	bool try_load_managed_mod_via_bridge(const std::string& mod_name, const std::filesystem::path& mod_directory, const std::filesystem::path& assembly_path);
	void load_managed_mod(const std::string& mod_name, const std::filesystem::path& mod_directory, const std::filesystem::path& managed_directory, const std::filesystem::path& assembly_path);
	void load_managed_mods_from_directory(const std::string& mod_name, const std::filesystem::path& mod_directory, const std::filesystem::path& managed_directory);

	events::EventManager* event_manager{nullptr};
	rml::dotnet::DotnetHost m_dotnet_host;
	std::vector<managed_mod_state> m_managed_mods;
	managed_bridge_state m_managed_bridge;
};

inline mod_manager* g_mod_manager{};
