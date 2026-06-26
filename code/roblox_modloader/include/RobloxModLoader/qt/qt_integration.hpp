#pragma once

#include "RobloxModLoader/qt/action_dispatcher.hpp"
#include "RobloxModLoader/qt/mods_menu.hpp"

namespace rml::qt
{
	class QtIntegration
	{
	public:
		QtIntegration();

		~QtIntegration();

		QtIntegration(const QtIntegration&) = delete;

		QtIntegration& operator=(const QtIntegration&) = delete;

		bool ensure_action_hook();

		[[nodiscard]] bool is_action_hook_ready() const;

		[[nodiscard]] ModsMenu& menu()
		{
			return m_menu;
		}
		void on_menu_bar_built(void* menu_bar)
		{
			m_menu.rebuild(menu_bar);
		}
		void on_action_triggered(void* action) const
		{
			m_dispatcher.dispatch(action);
		}

		[[nodiscard]] static QtIntegration* instance();

	private:
		ActionDispatcher m_dispatcher;
		ModsMenu m_menu;

		static inline QtIntegration* s_instance = nullptr;
	};
}
