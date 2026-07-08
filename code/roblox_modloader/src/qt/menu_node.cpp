#include "RobloxModLoader/qt/menu_node.hpp"

#include "RobloxModLoader/qt/mods_menu.hpp"

namespace rml::qt
{
	MenuNode::MenuNode(ModsMenu& menu, const uint64_t id) :
	    m_menu(&menu), m_id(id)
	{
	}

	MenuNode MenuNode::add_submenu(std::string text)
	{
		return MenuNode{*m_menu, m_menu->add_submenu(m_id, std::move(text))};
	}

	void MenuNode::add_action(std::string text, std::function<void()> on_click)
	{
		m_menu->add_action(m_id, std::move(text), std::move(on_click));
	}

	void MenuNode::add_separator()
	{
		m_menu->add_separator(m_id);
	}

	void MenuNode::add_checkable(std::string text, const bool initial, std::function<void(bool)> on_toggle)
	{
		m_menu->add_checkable(m_id, std::move(text), initial, std::move(on_toggle));
	}

	void MenuNode::set_icon(std::string path)
	{
		m_menu->set_item_icon(m_id, std::move(path));
	}
}
