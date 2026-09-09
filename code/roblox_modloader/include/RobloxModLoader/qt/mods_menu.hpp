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
	class MenuNode;
	class QAction;
	class QMenu;
	class QMenuBar;

	enum class MenuItemKind
	{
		Action,
		Submenu,
		Separator,
		CheckableAction,
	};

	class RML_EXPORT ModsMenu
	{
	public:
		explicit ModsMenu(ActionDispatcher& dispatcher);

		void add_action(std::string text, std::function<void()> on_click);

		uint64_t register_action(std::string text, std::function<void()> on_click);
		void remove_action(uint64_t id);

		uint64_t add_action(uint64_t parent_id, std::string text, std::function<void()> on_click);
		uint64_t add_submenu(uint64_t parent_id, std::string text);
		uint64_t add_separator(uint64_t parent_id);
		uint64_t add_checkable(uint64_t parent_id, std::string text, bool initial, std::function<void(bool)> on_toggle);
		void set_item_icon(uint64_t id, std::string path);
		void remove(uint64_t id);

		[[nodiscard]] rml::qt::MenuNode add_submenu(std::string text);
		[[nodiscard]] rml::qt::MenuNode root_node();

		void rebuild(QMenuBar* menu_bar);

	private:
		struct MenuItem
		{
			uint64_t id;
			uint64_t parent_id;
			MenuItemKind kind;
			std::string text;
			std::string icon_path;
			bool checked{false};
			std::function<void()> on_click;
			std::function<void(bool)> on_toggle;
			std::vector<uint64_t> children;
		};

		static void show_about();
		static void persist_hot_reload(bool enabled);

		uint64_t add_node(uint64_t parent_id, MenuItem node);
		void build_into(QMenu* parent_menu, uint64_t node_id, const std::unordered_map<uint64_t, MenuItem>& nodes, std::vector<QAction*>& live);
		void set_checked_state(uint64_t id, bool checked);
		void erase_subtree(uint64_t id);
		void trigger_rebuild();

		ActionDispatcher& m_dispatcher;

		std::mutex m_mutex;
		std::unordered_map<uint64_t, MenuItem> m_nodes;
		std::vector<uint64_t> m_root_children;
		std::vector<QAction*> m_live_actions;
		QMenuBar* m_menu_bar_handle{};
		QMenu* m_menu{};
		uint64_t m_next_id{1};
	};
}
