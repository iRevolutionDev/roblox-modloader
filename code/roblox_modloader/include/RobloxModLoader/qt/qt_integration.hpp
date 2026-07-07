#pragma once

#include "RobloxModLoader/qt/action_dispatcher.hpp"
#include "RobloxModLoader/qt/mods_menu.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class RML_EXPORT QtIntegration
	{
	public:
		QtIntegration();

		~QtIntegration();

		QtIntegration(const QtIntegration&) = delete;

		QtIntegration& operator=(const QtIntegration&) = delete;

		bool ensure_action_hook();

		[[nodiscard]] bool is_action_hook_ready() const;

		[[nodiscard]] ModsMenu& menu();

		void on_menu_bar_built(QMenuBar* menu_bar);

		void on_action_triggered(QAction* action) const;

		[[nodiscard]] static QtIntegration* instance();

	private:
		ActionDispatcher m_dispatcher;
		ModsMenu m_menu;

		static QtIntegration* s_instance;
	};
}
