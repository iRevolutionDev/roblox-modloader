#include "RobloxModLoader/qt/qmessagebox.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

#include <new>

namespace rml::qt
{
	constexpr std::size_t INSTANCE_SIZE = 48;

	QMessageBox* QMessageBox::create(QWidget* parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, void*)>("??0QMessageBox@@QEAA@PEAVQWidget@@@Z");
		if (!construct)
			return nullptr;

		void* memory = operator new(INSTANCE_SIZE);
		construct(memory, parent);
		return static_cast<QMessageBox*>(memory);
	}

	void QMessageBox::destroy(QMessageBox* box)
	{
		if (!box)
			return;

		static const auto dtor = detail::widgets<void (*)(void*)>("??1QMessageBox@@UEAA@XZ");
		if (dtor)
			dtor(box);
		::operator delete(box);
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
