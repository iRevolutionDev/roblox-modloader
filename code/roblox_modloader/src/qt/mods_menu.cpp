#include "RobloxModLoader/qt/mods_menu.hpp"

#include "RobloxModLoader/common.hpp"
#include "RobloxModLoader/qt/action_dispatcher.hpp"
#include "RobloxModLoader/qt/qmenu.hpp"
#include "RobloxModLoader/qt/qmenubar.hpp"
#include "RobloxModLoader/qt/qmessagebox.hpp"
#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/version.hpp"
#include "logo.hpp"
#include "utils/directory.hpp"
#include "utils/shell.hpp"

namespace rml::qt
{
	void ModsMenu::show_about()
	{
		QMessageBox* box = QMessageBox::create();

		if (!box)
		{
			utils::shell::message_box("About RobloxModLoader",
			    "RobloxModLoader\n\n"
			    "A modding framework for Roblox Studio — native, C#, and Luau mods.\n\n"
			    "github.com/revolutionxk/roblox-modloader");
			return;
		}

		const auto logo_path = utils::directory::get_mod_loader_directory() / "assets" / "logo.png";
		std::error_code ec;
		std::filesystem::create_directories(logo_path.parent_path(), ec);
		if (std::ofstream out{logo_path, std::ios::binary | std::ios::trunc})
			out.write(reinterpret_cast<const char*>(LOGO_PNG), LOGO_PNG_SIZE);

		const std::string logo_url = "file:///" + logo_path.generic_string();

		const std::string version = std::string(version::commit()) + (version::dirty() ? "-dirty" : "") + " &middot; " + version::branch() + " &middot; " + version::date();

		box->setWindowTitle("About RobloxModLoader");
		box->setTextFormat(TextFormat::RichText);
		box->addButton(QMessageBox::Ok);
		box->setStyleSheet("QMessageBox { background-color:#1e1f22; }"
		                   "QMessageBox QLabel { color:#e6e6e6; min-width:320px; }"
		                   "QPushButton { color:white; padding:6px 22px; font-weight:600; }");

		const std::string html = "<div align='center' style='font-family:Segoe UI, Arial'>"
		                         "<img src='"
		    + logo_url
		    + "' width='96' height='96'><br>"
		      "<span style='font-size:20px; font-weight:600;'>Roblox Mod Loader</span><br>"
		      "<span style='color:#9aa0a6'>A modding framework for Roblox Studio</span>"
		      "<p style='color:#dddddd; margin-top:12px'>Native, C# and Luau mods loaded directly into Studio.</p>"
		      "<a href='https://github.com/revolutionxk/roblox-modloader' style='color:#5b9bff'>"
		      "github.com/revolutionxk/roblox-modloader</a>"
		      "<p style='color:#5c6066; font-size:11px; margin-top:14px'>"
		    + version
		    + "</p>"
		      "</div>";
		box->setText(QString{std::string_view{html}});
		box->exec();

		QMessageBox::destroy(box);
	}

	ModsMenu::ModsMenu(ActionDispatcher& dispatcher) :
	    m_dispatcher(dispatcher)
	{
	}

	void ModsMenu::add_action(std::string text, std::function<void()> on_click)
	{
		if (text.empty() || !on_click)
			return;

		std::scoped_lock lock(m_mutex);
		m_entries.push_back({m_next_id++, std::move(text), std::move(on_click)});
	}

	uint64_t ModsMenu::register_action(std::string text, std::function<void()> on_click)
	{
		if (text.empty() || !on_click)
			return 0;

		void* menu_bar = nullptr;
		uint64_t id = 0;
		{
			std::scoped_lock lock(m_mutex);
			id = m_next_id++;
			m_entries.push_back({id, std::move(text), std::move(on_click)});
			menu_bar = m_menu_bar_handle;
		}

		if (menu_bar)
			rebuild(menu_bar);

		return id;
	}

	void ModsMenu::remove_action(uint64_t id)
	{
		if (id == 0)
			return;

		void* live_action = nullptr;
		{
			std::scoped_lock lock(m_mutex);
			std::erase_if(m_entries, [id](const Entry& entry) { return entry.id == id; });
			if (const auto it = m_entry_actions.find(id); it != m_entry_actions.end())
			{
				live_action = it->second;
				m_entry_actions.erase(it);
			}
		}
		
		if (live_action)
			m_dispatcher.disconnect(live_action);
	}

	void ModsMenu::rebuild(void* menu_bar_handle)
	{
		if (!menu_bar_handle)
			return;

		std::vector<Entry> entries;
		std::vector<void*> previous;
		QMenu* menu = nullptr;
		{
			std::scoped_lock lock(m_mutex);
			entries = m_entries;
			previous = std::move(m_live_actions);
			m_live_actions.clear();
			if (m_menu && m_menu_bar_handle == menu_bar_handle)
				menu = m_menu;
		}

		for (void* stale : previous)
			m_dispatcher.disconnect(stale);

		if (menu)
		{
			menu->clear();
		}
		else
		{
			auto* const menu_bar = static_cast<QMenuBar*>(menu_bar_handle);
			menu = menu_bar->addMenu("Mods");
			if (!menu)
			{
				LOG_ERROR("[qt] QMenuBar::addMenu returned null; Mods menu not added");
				return;
			}
		}

		std::vector<void*> live;
		std::unordered_map<uint64_t, void*> entry_actions;

		const auto emit = [&](const std::string_view text, std::function<void()> on_click, const uint64_t id = 0) {
			QAction* const action = menu->addAction(text);
			if (!action)
				return;

			m_dispatcher.connect(action->handle(), std::move(on_click));
			live.push_back(action->handle());
			if (id != 0)
				entry_actions[id] = action->handle();
		};

		emit("Open Mods Folder", [] {
			utils::shell::open_folder(utils::directory::get_mod_loader_directory() / "mods");
		});
		emit("Open Logs Folder", [] {
			utils::shell::open_folder(utils::directory::get_mod_loader_directory() / "logs");
		});
		emit("Open Config", [] {
			utils::shell::open(utils::directory::get_mod_loader_directory() / "config.toml");
		});

		if (!entries.empty())
		{
			menu->addSeparator();
			for (auto& [id, text, on_click] : entries)
				emit(text, on_click, id);
		}

		menu->addSeparator();
		emit("About", [] {
			show_about();
		});

		{
			std::scoped_lock lock(m_mutex);
			m_menu_bar_handle = menu_bar_handle;
			m_menu = menu;
			m_live_actions = std::move(live);
			m_entry_actions = std::move(entry_actions);
		}

		LOG_INFO("[qt] Mods menu rebuilt ({} mod entr(ies) + built-ins)", entries.size());
	}
}
