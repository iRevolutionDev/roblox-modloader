#include "RobloxModLoader/qt/qpushbutton.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QPushButton* QPushButton::create(const std::string_view text, QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, const void*, void*)>("??0QPushButton@@QEAA@AEBVQString@@PEAVQWidget@@@Z");
		if (!construct)
			return nullptr;

		const QString label(text);
		return detail::heap_construct<QPushButton>(detail::WIDGET_INSTANCE_SIZE, construct, label.data(), parent);
	}

	QtOwned<QPushButton> QPushButton::create_owned(const std::string_view text)
	{
		return QtOwned<QPushButton>(create(text, nullptr));
	}

	void QPushButton::destroy(QPushButton* button)
	{
		static const auto dtor = detail::widgets<void (*)(void*)>("??1QPushButton@@UEAA@XZ");
		detail::heap_destroy(dtor, button);
	}
}
