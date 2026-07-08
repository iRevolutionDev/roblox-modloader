#pragma once

#include "RobloxModLoader/rml_export.hpp"

#include <cstdint>
#include <functional>
#include <string>

namespace rml::qt
{
	class ModsMenu;

	class RML_EXPORT MenuNode
	{
	public:
		MenuNode(ModsMenu& menu, uint64_t id);

		MenuNode(const MenuNode&) = delete;
		MenuNode& operator=(const MenuNode&) = delete;
		MenuNode(MenuNode&&) = default;
		MenuNode& operator=(MenuNode&&) = default;

		[[nodiscard]] MenuNode add_submenu(std::string text);
		void add_action(std::string text, std::function<void()> on_click);
		void add_separator();
		void add_checkable(std::string text, bool initial, std::function<void(bool)> on_toggle);
		void set_icon(std::string path);

	private:
		ModsMenu* m_menu;
		uint64_t m_id;
	};
}
