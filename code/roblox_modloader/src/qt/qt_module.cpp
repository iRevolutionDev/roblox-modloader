#include "RobloxModLoader/qt/qt_module.hpp"

#include "RobloxModLoader/internal/common.hpp"
#include "RobloxModLoader/memory/module.hpp"

#include <initializer_list>
#include <memory>
#include <mutex>

RML_LOG_SCOPE("Qt");

namespace rml::qt::detail
{
	struct QtLibrary
	{
		std::initializer_list<const char*> m_candidates;
		std::unique_ptr<memory::module> m_module;
		std::mutex m_mutex;

		memory::module* resolve()
		{
			std::scoped_lock lock(m_mutex);

			if (m_module && m_module->loaded())
				return m_module.get();

			for (const char* name : m_candidates)
			{
				auto candidate = std::make_unique<memory::module>(std::string_view{name});
				if (candidate->loaded())
				{
					m_module = std::move(candidate);
					return m_module.get();
				}
			}

			return nullptr;
		}
	};
	
	static QtLibrary g_core{{"Qt5Core.dll", "Qt5Cored.dll", "QtCore"}, nullptr, {}};
	static QtLibrary g_widgets{{"Qt5Widgets.dll", "Qt5Widgetsd.dll", "QtWidgets"}, nullptr, {}};
	static QtLibrary g_gui{{"Qt5Gui.dll", "Qt5Guid.dll", "QtGui"}, nullptr, {}};

	static void* find_quiet(QtLibrary& library, const char* signature)
	{
		memory::module* qt = library.resolve();
		return qt ? qt->find_export(signature).as<void*>() : nullptr;
	}

	static void* find(QtLibrary& library, std::initializer_list<const char*> signatures)
	{
		memory::module* qt = library.resolve();
		if (!qt)
			return nullptr;

		for (const char* signature : signatures)
		{
			if (void* address = qt->find_export(signature).as<void*>())
				return address;
		}

		RML_WARN("Qt symbol not found: {}", *signatures.begin());
		return nullptr;
	}

	void* core_export_optional(const char* signature)
	{
		return find_quiet(g_core, signature);
	}

	void* widgets_export_optional(const char* signature)
	{
		return find_quiet(g_widgets, signature);
	}

	void* gui_export_optional(const char* signature)
	{
		return find_quiet(g_gui, signature);
	}

	void* core_export(const char* signature)
	{
		return find(g_core, {signature});
	}

	void* widgets_export(const char* signature)
	{
		return find(g_widgets, {signature});
	}

	void* gui_export(const char* signature)
	{
		return find(g_gui, {signature});
	}

	void* core_export(std::initializer_list<const char*> signatures)
	{
		return find(g_core, signatures);
	}

	void* widgets_export(std::initializer_list<const char*> signatures)
	{
		return find(g_widgets, signatures);
	}

	void* gui_export(std::initializer_list<const char*> signatures)
	{
		return find(g_gui, signatures);
	}
}
