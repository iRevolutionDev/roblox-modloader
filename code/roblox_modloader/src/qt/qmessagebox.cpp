#include "RobloxModLoader/qt/qmessagebox.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QMessageBox* QMessageBox::create(QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, void*)>("??0QMessageBox@@QEAA@PEAVQWidget@@@Z");
		return detail::heap_construct<QMessageBox>(detail::WIDGET_INSTANCE_SIZE, construct, parent);
	}

	QtOwned<QMessageBox> QMessageBox::create_owned()
	{
		return QtOwned<QMessageBox>(create(nullptr));
	}

	void QMessageBox::destroy(QMessageBox* box)
	{
		static const auto dtor = detail::widgets<void (*)(void*)>("??1QMessageBox@@UEAA@XZ");
		detail::heap_destroy(dtor, box);
	}

	void QMessageBox::setText(const QString& text)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setText@QMessageBox@@QEAAXAEBVQString@@@Z");
		if (fn)
			fn(this, text.data());
	}

	void QMessageBox::setIcon(const Icon icon)
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setIcon@QMessageBox@@QEAAXW4Icon@1@@Z");
		if (fn)
			fn(this, icon);
	}

	void QMessageBox::setTextFormat(const TextFormat format)
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setTextFormat@QMessageBox@@QEAAXW4TextFormat@Qt@@@Z");
		if (fn)
			fn(this, static_cast<int>(format));
	}

	QPushButton* QMessageBox::addButton(const StandardButton button)
	{
		static const auto fn = detail::widgets<void* (*)(void*, int)>("?addButton@QMessageBox@@QEAAPEAVQPushButton@@W4StandardButton@1@@Z");
		return fn ? static_cast<QPushButton*>(fn(this, button)) : nullptr;
	}
}
