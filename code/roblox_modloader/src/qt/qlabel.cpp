#include "RobloxModLoader/qt/qlabel.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	namespace
	{
		constexpr std::size_t INSTANCE_SIZE = 128;
	}

	QLabel* QLabel::create(const std::string_view text, QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, const void*, void*, int)>("??0QLabel@@QEAA@AEBVQString@@PEAVQWidget@@V?$QFlags@W4WindowType@Qt@@@@@Z");
		if (!construct)
			return nullptr;

		const QString label(text);
		void* memory = operator new(INSTANCE_SIZE);
		construct(memory, label.data(), parent, 0);
		return static_cast<QLabel*>(memory);
	}

	void QLabel::destroy(QLabel* label)
	{
		if (!label)
			return;

		static const auto dtor = detail::widgets<void (*)(void*)>("??1QLabel@@UEAA@XZ");
		if (dtor)
			dtor(label);
		operator delete(label);
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
