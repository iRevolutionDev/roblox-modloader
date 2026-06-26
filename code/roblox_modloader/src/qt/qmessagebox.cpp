#include "RobloxModLoader/qt/qmessagebox.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

#include <new>

namespace rml::qt
{
	constexpr std::size_t INSTANCE_SIZE = 48;

	QMessageBox::QMessageBox(const QWidget& parent)
	{
		static const auto construct = detail::widgets<void* (*)(void*, void*)>("??0QMessageBox@@QEAA@PEAVQWidget@@@Z");
		if (!construct)
			return;

		m_this = operator new(INSTANCE_SIZE);
		construct(m_this, parent.handle());
		m_owned = true;
	}

	QMessageBox::~QMessageBox()
	{
		if (!m_owned || !m_this)
			return;

		static const auto destroy = detail::widgets<void (*)(void*)>("??1QMessageBox@@UEAA@XZ");
		if (destroy)
			destroy(m_this);

		::operator delete(m_this);
	}

	void QMessageBox::setText(const QString& text) const
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setText@QMessageBox@@QEAAXAEBVQString@@@Z");
		if (fn && m_this)
			fn(m_this, text.data());
	}

	void QMessageBox::setIcon(const Icon icon) const
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setIcon@QMessageBox@@QEAAXW4Icon@1@@Z");
		if (fn && m_this)
			fn(m_this, icon);
	}

	void QMessageBox::setTextFormat(const TextFormat format) const
	{
		static const auto fn = detail::widgets<void (*)(void*, int)>("?setTextFormat@QMessageBox@@QEAAXW4TextFormat@Qt@@@Z");
		if (fn && m_this)
			fn(m_this, static_cast<int>(format));
	}

	QPushButton QMessageBox::addButton(const StandardButton button) const
	{
		static const auto fn = detail::widgets<void* (*)(void*, int)>("?addButton@QMessageBox@@QEAAPEAVQPushButton@@W4StandardButton@1@@Z");
		return (fn && m_this) ? QPushButton{fn(m_this, button)} : QPushButton{};
	}
}
