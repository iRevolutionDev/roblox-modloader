#include "RobloxModLoader/qt/qpalette.hpp"

#include "RobloxModLoader/qt/qbrush.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QPalette::QPalette(const void* source)
	{
		static const auto copy_ctor = detail::gui<void (*)(void*, const void*)>("??0QPalette@@QEAA@AEBV0@@Z");
		if (!copy_ctor || !source)
			return;

		copy_ctor(m_storage, source);
		m_owned = true;
	}

	QPalette::QPalette(QPalette&& other) noexcept
	{
		std::memcpy(m_storage, other.m_storage, sizeof(m_storage));
		m_owned = other.m_owned;
		other.m_owned = false;
	}

	QPalette& QPalette::operator=(QPalette&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			std::memcpy(m_storage, other.m_storage, sizeof(m_storage));
			m_owned = other.m_owned;
			other.m_owned = false;
		}
		return *this;
	}

	QPalette::~QPalette()
	{
		destroy();
	}

	void QPalette::destroy()
	{
		if (!m_owned)
			return;

		static const auto dtor = detail::gui<void (*)(void*)>("??1QPalette@@QEAA@XZ");
		if (dtor)
			dtor(m_storage);
		m_owned = false;
	}

	void QPalette::set_brush(const ColorRole role, const QBrush& brush)
	{
		static const auto fn = detail::gui<void (*)(void*, int, const void*)>("?setBrush@QPalette@@QEAAXW4ColorRole@1@AEBVQBrush@@@Z");
		if (fn && m_owned)
			fn(m_storage, static_cast<int>(role), brush.data());
	}
}
