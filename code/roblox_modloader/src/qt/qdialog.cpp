#include "RobloxModLoader/qt/qdialog.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

#include <new>

namespace rml::qt
{
	namespace
	{
		constexpr std::size_t INSTANCE_SIZE = 128;
	}

	QDialog* QDialog::create(QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, void*, int)>("??0QDialog@@QEAA@PEAVQWidget@@V?$QFlags@W4WindowType@Qt@@@@@Z");
		if (!construct)
			return nullptr;

		void* memory = ::operator new(INSTANCE_SIZE);
		construct(memory, parent, 0);
		return static_cast<QDialog*>(memory);
	}

	void QDialog::destroy(QDialog* dialog)
	{
		if (!dialog)
			return;

		static const auto dtor = detail::widgets<void (*)(void*)>("??1QDialog@@UEAA@XZ");
		if (dtor)
			dtor(dialog);
		::operator delete(dialog);
	}

	void QDialog::setModal(const bool modal)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("?setModal@QDialog@@QEAAX_N@Z");
		if (fn)
			fn(this, modal);
	}

	int QDialog::exec()
	{
		static const auto fn = detail::widgets<int (*)(void*)>("?exec@QDialog@@UEAAHXZ");
		return fn ? fn(this) : -1;
	}
}
