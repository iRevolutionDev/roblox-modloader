#include "RobloxModLoader/mod/mod_base.hpp"

#include <RobloxModLoader/logger/logger.hpp>
#include <RobloxModLoader/qt/menu_node.hpp>
#include <RobloxModLoader/qt/qt_integration.hpp>
#include <spdlog/spdlog.h>

namespace menu_showcase
{
	class MenuShowcase final : public ModBase
	{
	public:
		MenuShowcase()
		{
			name = "Menu Showcase";
			version = "1.0.0";
			author = "RobloxModLoader";
			description = "Demonstrates the fluent mods-menu tree API: submenus, actions, separators, checkables and icons.";
			m_log = rml::Logger::get_logger("MenuShowcase");
		}

		void on_load() override
		{
			rml::qt::QtIntegration* const qt = rml::qt::QtIntegration::instance();
			if (!qt)
			{
				m_log->warn("Qt integration unavailable; the mods menu will not be built");
				return;
			}

			rml::qt::MenuNode root = qt->menu().add_submenu("Menu Showcase");
			root.set_icon((paths().root() / "icon.png").string());

			root.add_action("Say Hello", [this] {
				m_log->info("Hello from Menu Showcase!");
			});

			root.add_separator();

			root.add_checkable("Enable Thing", true, [this](const bool enabled) {
				m_log->info("Enable Thing toggled to {}", enabled);
			});

			rml::qt::MenuNode advanced = root.add_submenu("Advanced");
			advanced.add_action("Reset", [this] {
				m_log->info("Reset clicked");
			});

			m_log->info("loaded");
		}

		void on_unload() override
		{
			m_log->info("unloaded");
		}

	private:
		std::shared_ptr<spdlog::logger> m_log;
	};
}

#define MENU_SHOWCASE_MOD_API __declspec(dllexport)

extern "C"
{
	MENU_SHOWCASE_MOD_API ModBase* start_mod()
	{
		return new menu_showcase::MenuShowcase();
	}

	MENU_SHOWCASE_MOD_API void uninstall_mod(const ModBase* mod)
	{
		delete mod;
	}
}

RML_EXPORT_MOD_ABI_VERSION()
