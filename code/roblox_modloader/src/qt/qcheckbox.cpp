#include "RobloxModLoader/qt/qcheckbox.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	namespace
	{
		constexpr std::size_t INSTANCE_SIZE = 128;
	}

	QCheckBox* QCheckBox::create(const std::string_view text, QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, const void*, void*)>("??0QCheckBox@@QEAA@AEBVQString@@PEAVQWidget@@@Z");
		if (!construct)
			return nullptr;

		const QString label(text);
		void* memory = ::operator new(INSTANCE_SIZE);
		construct(memory, label.data(), parent);
		return static_cast<QCheckBox*>(memory);
	}

	void QCheckBox::destroy(QCheckBox* box)
	{
		if (!box)
			return;

		static const auto dtor = detail::widgets<void (*)(void*)>("??1QCheckBox@@UEAA@XZ");
		if (dtor)
			dtor(box);
		::operator delete(box);
	}
}
