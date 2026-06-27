#include "RobloxModLoader/qt/qpixmap.hpp"

#include "RobloxModLoader/qt/qcolor.hpp"
#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QPixmap::QPixmap(const std::string_view file_path)
	{
		static const auto ctor = detail::gui<void (*)(void*, const void*, const char*, int)>("??0QPixmap@@QEAA@AEBVQString@@PEBDV?$QFlags@W4ImageConversionFlag@Qt@@@@@Z");
		if (!ctor)
			return;

		const QString path(file_path);
		ctor(m_storage, path.data(), nullptr, 0);
		m_loaded = true;
	}

	QPixmap::QPixmap(const int width, const int height)
	{
		static const auto ctor = detail::gui<void (*)(void*, int, int)>("??0QPixmap@@QEAA@HH@Z");
		if (!ctor)
			return;
		ctor(m_storage, width, height);
		m_loaded = true;
	}

	int QPixmap::width() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("?width@QPixmap@@QEBAHXZ");
		return fn ? fn(m_storage) : 0;
	}

	int QPixmap::height() const
	{
		static const auto fn = detail::gui<int (*)(const void*)>("?height@QPixmap@@QEBAHXZ");
		return fn ? fn(m_storage) : 0;
	}

	void QPixmap::fill(const QColor& color) const
	{
		static const auto fn = detail::gui<void (*)(const void*, const void*)>("?fill@QPixmap@@QEAAXAEBVQColor@@@Z");
		if (fn)
			fn(m_storage, color.data());
	}

	bool QPixmap::save(const std::string_view file_path) const
	{
		static const auto fn = detail::gui<bool (*)(const void*, const void*, const char*, int)>("?save@QPixmap@@QEBA_NAEBVQString@@PEBDH@Z");
		if (!fn)
			return false;
		const QString path(file_path);
		return fn(m_storage, path.data(), nullptr, -1);
	}
}
