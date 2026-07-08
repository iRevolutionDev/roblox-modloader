#include "RobloxModLoader/qt/qdialog.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QDialog* QDialog::create(QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, void*, int)>("??0QDialog@@QEAA@PEAVQWidget@@V?$QFlags@W4WindowType@Qt@@@@@Z");
		return detail::heap_construct<QDialog>(detail::WIDGET_INSTANCE_SIZE, construct, parent, 0);
	}

	QtOwned<QDialog> QDialog::create_owned()
	{
		return QtOwned<QDialog>(create(nullptr));
	}

	void QDialog::destroy(QDialog* dialog)
	{
		static const auto dtor = detail::widgets<void (*)(void*)>("??1QDialog@@UEAA@XZ");
		detail::heap_destroy(dtor, dialog);
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
