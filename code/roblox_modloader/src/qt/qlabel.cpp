#include "RobloxModLoader/qt/qlabel.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QLabel* QLabel::create(const std::string_view text, QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, const void*, void*, int)>("??0QLabel@@QEAA@AEBVQString@@PEAVQWidget@@V?$QFlags@W4WindowType@Qt@@@@@Z");
		if (!construct)
			return nullptr;

		const QString label(text);
		return detail::heap_construct<QLabel>(detail::WIDGET_INSTANCE_SIZE, construct, label.data(), parent, 0);
	}

	QtOwned<QLabel> QLabel::create_owned(const std::string_view text)
	{
		return QtOwned<QLabel>(create(text, nullptr));
	}

	void QLabel::destroy(QLabel* label)
	{
		static const auto dtor = detail::widgets<void (*)(void*)>("??1QLabel@@UEAA@XZ");
		detail::heap_destroy(dtor, label);
	}

	void QLabel::setText(const std::string_view text)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setText@QLabel@@QEAAXAEBVQString@@@Z");
		if (fn)
		{
			const QString value(text);
			fn(this, value.data());
		}
	}

	void QLabel::setAlignment(const int alignment)
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setAlignment@QLabel@@QEAAXV?$QFlags@W4AlignmentFlag@Qt@@@@@Z");
		if (fn)
			fn(this, alignment);
	}
}
