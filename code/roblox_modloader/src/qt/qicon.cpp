#include "RobloxModLoader/qt/qicon.hpp"

#include "RobloxModLoader/qt/qstring.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	QIcon::QIcon(const QString& path)
	{
		static const auto ctor = detail::gui<void (*)(void*, const void*)>("??0QIcon@@QEAA@AEBVQString@@@Z");
		if (!ctor)
			return;

		ctor(m_storage, path.data());
		set_owned(true);
	}

	QIcon::~QIcon()
	{
		if (!owned())
			return;

		static const auto dtor = detail::gui<void (*)(void*)>("??1QIcon@@QEAA@XZ");
		if (dtor)
			dtor(m_storage);
	}
}
