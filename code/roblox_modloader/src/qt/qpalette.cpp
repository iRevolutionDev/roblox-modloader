#include "RobloxModLoader/qt/qpalette.hpp"

#include "RobloxModLoader/qt/qbrush.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QPalette::QPalette(const void* source)
	{
		static const auto copy_ctor = detail::gui<void (*)(void*, const void*)>("QPalette::QPalette(QPalette const&)");
		if (!copy_ctor || !source)
			return;

		copy_ctor(m_storage, source);
		set_owned(true);
	}

	QPalette::QPalette(QPalette&& other) noexcept
	{
		adopt(other);
	}

	QPalette& QPalette::operator=(QPalette&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			adopt(other);
		}
		return *this;
	}

	QPalette::~QPalette()
	{
		destroy();
	}

	void QPalette::destroy()
	{
		if (!owned())
			return;

		static const auto dtor = detail::gui<void (*)(void*)>("QPalette::~QPalette()");
		if (dtor)
			dtor(m_storage);
		set_owned(false);
	}

	void QPalette::set_brush(const ColorRole role, const QBrush& brush)
	{
		constexpr int all_color_groups = 5;

		static const auto fn = detail::gui<void (*)(void*, int, int, const void*)>("QPalette::setBrush(QPalette::ColorGroup, QPalette::ColorRole, QBrush const&)");
		if (fn && owned())
			fn(m_storage, all_color_groups, static_cast<int>(role), brush.data());
	}
}
