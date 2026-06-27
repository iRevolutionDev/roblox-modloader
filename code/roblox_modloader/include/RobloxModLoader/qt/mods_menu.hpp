#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <functional>
#include <mutex>
#include <string>
#include <vector>

namespace rml::qt
{
	class ActionDispatcher;

	class RML_EXPORT ModsMenu
	{
	public:
		explicit ModsMenu(ActionDispatcher& dispatcher);

		void add_action(std::string text, std::function<void()> on_click);
		void rebuild(void* menu_bar);

	private:
		struct Entry
		{
			std::string text;
			std::function<void()> on_click;
		};

		static void show_about();

		ActionDispatcher& m_dispatcher;

		std::mutex m_mutex;
		std::vector<Entry> m_entries;
		std::vector<void*> m_live_actions;
	};
}
