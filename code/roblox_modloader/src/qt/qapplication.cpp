#include "RobloxModLoader/qt/qapplication.hpp"

#include "RobloxModLoader/memory/foreign_call.hpp"
#include "RobloxModLoader/qt/qarray_data.hpp"
#include "RobloxModLoader/qt/qlist.hpp"
#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	QApplication* QApplication::instance()
	{
		static void* const fn = detail::core_export_optional("QCoreApplication::instance()");
		if (fn)
			return static_cast<QApplication*>(reinterpret_cast<void* (*)()>(fn)());

		static auto* const self = static_cast<void* const*>(detail::core_export("QCoreApplication::self"));
		return self ? static_cast<QApplication*>(*self) : nullptr;
	}

	void QApplication::set_style_sheet(const std::string_view css)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("QApplication::setStyleSheet(QString const&)");
		if (!fn)
			return;
		const QString style(css);
		fn(this, style.data());
	}

	std::string QApplication::style_sheet() const
	{
		static void* const get = detail::widgets_export("QApplication::styleSheet() const");
		static void* const to_utf8 = detail::core_export("QString::toUtf8() const");
		
		static const auto const_data = detail::core_optional<const char* (*)(const void* self)>("QByteArray::constData() const");

		if (!get || !to_utf8)
			return {};

		void* qstring = nullptr;
		void* qbytearray = nullptr;
		memory::call_returning_member(get, qstring, static_cast<const void*>(this));
		memory::call_returning_member(to_utf8, qbytearray, static_cast<const void*>(&qstring));

		const char* const utf8 = const_data ? const_data(&qbytearray) : detail::array_data_begin(qbytearray);
		std::string result = utf8 ? utf8 : "";

		detail::destroy_qbytearray(qbytearray);
		detail::destroy_qstring(qstring);
		return result;
	}

	std::vector<QWidget*> QApplication::all_widgets()
	{
		static void* const fn = detail::widgets_export("QApplication::allWidgets()");

		std::vector<QWidget*> widgets;
		if (!fn)
			return widgets;

		QList<QWidget*> list;
		memory::call_returning(fn, *list.raw_storage());

		widgets.reserve(static_cast<std::size_t>(list.size()));
		for (QWidget* widget : list)
			widgets.push_back(widget);

		return widgets;
	}
}
