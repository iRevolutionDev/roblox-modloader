#pragma once
#include "RobloxModLoader/common.hpp"
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

	void set_event_manager(events::EventManager* manager);

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
	void register_event_handler(events::EventManager::EventHandler<T> handler)
	{
		if (event_manager)
		{
			event_manager->registerHandler<T>(handler);
		}
	}

private:
	events::EventManager* event_manager{nullptr};
	rml::mod::ModPaths m_paths{};
};
