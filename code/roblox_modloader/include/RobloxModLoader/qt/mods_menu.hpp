#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace rml::qt
{
	class ActionDispatcher;
	class QAction;
	class QMenu;
	class QMenuBar;

	class RML_EXPORT ModsMenu
	{
	public:
		explicit ModsMenu(ActionDispatcher& dispatcher);

		void add_action(std::string text, std::function<void()> on_click);

		uint64_t register_action(std::string text, std::function<void()> on_click);
		void remove_action(uint64_t id);

		void rebuild(QMenuBar* menu_bar);

	private:
		struct Entry
		{
			uint64_t id;
			std::string text;
			std::function<void()> on_click;
		};

		static void show_about();

		ActionDispatcher& m_dispatcher;

		std::mutex m_mutex;
		std::vector<Entry> m_entries;
		std::vector<QAction*> m_live_actions;
		std::unordered_map<uint64_t, QAction*> m_entry_actions;
		QMenuBar* m_menu_bar_handle{};
		QMenu* m_menu{};
		uint64_t m_next_id{1};
	};
}
