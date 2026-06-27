#include "RobloxModLoader/qt/qpushbutton.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	namespace
	{
		constexpr std::size_t INSTANCE_SIZE = 128;
	}

	QPushButton* QPushButton::create(const std::string_view text, QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, const void*, void*)>("??0QPushButton@@QEAA@AEBVQString@@PEAVQWidget@@@Z");
		if (!construct)
			return nullptr;

		const QString label(text);
		void* memory = ::operator new(INSTANCE_SIZE);
		construct(memory, label.data(), parent);
		return static_cast<QPushButton*>(memory);
	}

	void QPushButton::destroy(QPushButton* button)
	{
		if (!button)
			return;

		static const auto dtor = detail::widgets<void (*)(void*)>("??1QPushButton@@UEAA@XZ");
		if (dtor)
			dtor(button);
		::operator delete(button);
	}
}
