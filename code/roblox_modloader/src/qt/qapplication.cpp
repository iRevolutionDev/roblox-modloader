#include "RobloxModLoader/qt/qapplication.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"
#include "RobloxModLoader/qt/qwidget.hpp"

namespace rml::qt
{
	QApplication* QApplication::instance()
	{
		static const auto fn = detail::core<void* (*)()>("?instance@QCoreApplication@@SAPEAV1@XZ");
		return fn ? static_cast<QApplication*>(fn()) : nullptr;
	}

	void QApplication::set_style_sheet(const std::string_view css)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setStyleSheet@QApplication@@QEAAXAEBVQString@@@Z");
		if (!fn)
			return;
		const QString style(css);
		fn(this, style.data());
	}

	std::string QApplication::style_sheet() const
	{
		static const auto get = detail::widgets<void (*)(const void* self, void* sret)>("?styleSheet@QApplication@@QEBA?AVQString@@XZ");
		static const auto to_utf8 = detail::core<void (*)(const void* self, void* sret)>("?toUtf8@QString@@QEBA?AVQByteArray@@XZ");
		static const auto const_data = detail::core<const char* (*)(const void* self)>("?constData@QByteArray@@QEBAPEBDXZ");
		static const auto string_dtor = detail::core<void (*)(void*)>("??1QString@@QEAA@XZ");
		static const auto bytes_dtor = detail::core<void (*)(void*)>("??1QByteArray@@QEAA@XZ");

		if (!get || !to_utf8 || !const_data)
			return {};

		void* qstring = nullptr;
		void* qbytearray = nullptr;
		get(this, &qstring);
		to_utf8(&qstring, &qbytearray);

		const char* utf8 = const_data(&qbytearray);
		std::string result = utf8 ? utf8 : "";

		if (bytes_dtor)
			bytes_dtor(&qbytearray);
		if (string_dtor)
			string_dtor(&qstring);
		return result;
	}

	std::vector<QWidget*> QApplication::all_widgets()
	{
		static const auto fn = detail::widgets<void (*)(void* sret)>("?allWidgets@QApplication@@SA?AV?$QList@PEAVQWidget@@@@XZ");

		std::vector<QWidget*> widgets;
		if (!fn)
			return widgets;

		void* list_d = nullptr;
		fn(&list_d);
		if (!list_d)
			return widgets;

		const auto* base = static_cast<unsigned char*>(list_d);
		const int begin = *reinterpret_cast<const int*>(base + 8);
		const int end = *reinterpret_cast<const int*>(base + 12);
		auto* const* array = reinterpret_cast<void* const*>(base + 16);

		if (end > begin)
		{
			widgets.reserve(static_cast<std::size_t>(end - begin));
			for (int i = begin; i < end; ++i)
				widgets.push_back(static_cast<QWidget*>(array[i]));
		}

		return widgets;
	}
}
