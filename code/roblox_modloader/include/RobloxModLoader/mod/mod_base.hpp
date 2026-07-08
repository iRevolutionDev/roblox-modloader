#pragma once
#include "RobloxModLoader/rml_export.hpp"
#include "RobloxModLoader/version.hpp"
#include "events.hpp"
#include "mod_paths.hpp"

#include <string>
#include <utility>

class RML_EXPORT ModBase
{
public:
	using start_type = ModBase* (*)();

	std::string name{};
	std::string version{};
	std::string author{};
	std::string description{};

	ModBase();

	virtual ~ModBase();

	virtual void on_load() = 0;

	virtual void on_unload() = 0;

	virtual void on_script_manager_load()
	{
	}

	void set_event_manager(rml::events::EventManager& manager);

	void set_paths(rml::mod::ModPaths paths)
	{
		m_paths = std::move(paths);
	}

	[[nodiscard]] const rml::mod::ModPaths& paths() const
	{
		return m_paths;
	}

	[[nodiscard]] const std::filesystem::path& mod_folder() const
	{
		return m_paths.root();
	}

protected:
	template<typename T>
	void register_event_handler(rml::events::EventManager::EventHandler<T> handler)
	{
		if (m_event_manager)
		{
			m_event_manager->register_handler<T>(handler);
		}
	}

private:
	rml::events::EventManager* m_event_manager{nullptr};
	rml::mod::ModPaths m_paths{};
};

using rml_abi_version_type = int (*)();

#if defined(_WIN32)
	#define RML_MOD_ABI_EXPORT __declspec(dllexport)
#else
	#define RML_MOD_ABI_EXPORT __attribute__((visibility("default")))
#endif

#define RML_EXPORT_MOD_ABI_VERSION() \
	extern "C" RML_MOD_ABI_EXPORT int rml_abi_version() { return RML_ABI_VERSION; }
