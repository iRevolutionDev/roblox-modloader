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
		m_entries.push_back({std::move(text), std::move(on_click)});
	}

	void ModsMenu::rebuild(void* menu_bar_handle)
	{
		if (!menu_bar_handle)
			return;

		std::vector<Entry> entries;
		std::vector<void*> previous;
		{
			std::scoped_lock lock(m_mutex);
			entries = m_entries;
			previous = std::move(m_live_actions);
			m_live_actions.clear();
		}

		for (void* stale : previous)
			m_dispatcher.disconnect(stale);

		auto* const menu_bar = static_cast<QMenuBar*>(menu_bar_handle);
		QMenu* const menu = menu_bar->addMenu("Mods");
		if (!menu)
		{
			LOG_ERROR("[qt] QMenuBar::addMenu returned null; Mods menu not added");
			return;
		}

		std::vector<void*> live;

		const auto emit = [&](const std::string_view text, std::function<void()> on_click) {
			QAction* const action = menu->addAction(text);
			if (!action)
				return;

			m_dispatcher.connect(action->handle(), std::move(on_click));
			live.push_back(action->handle());
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
			for (auto& [text, on_click] : entries)
				emit(text, on_click);
		}

		menu->addSeparator();
		emit("About", [] {
			show_about();
		});

		{
			std::scoped_lock lock(m_mutex);
			m_live_actions = std::move(live);
		}

		LOG_INFO("[qt] Mods menu rebuilt ({} mod entr(ies) + built-ins)", entries.size());
	}
}
