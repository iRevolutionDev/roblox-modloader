#include "RobloxModLoader/qt/qaction.hpp"

#include "RobloxModLoader/qt/qicon.hpp"
#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	void QAction::setCheckable(const bool checkable)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("?setCheckable@QAction@@QEAAX_N@Z");
		if (fn)
			fn(this, checkable);
	}

	void QAction::setChecked(const bool checked)
	{
		static const auto fn = detail::widgets<void (*)(void*, bool)>("?setChecked@QAction@@QEAAX_N@Z");
		if (fn)
			fn(this, checked);
	}

	bool QAction::isChecked() const
	{
		static const auto fn = detail::widgets<bool (*)(const void*)>("?isChecked@QAction@@QEBA_NXZ");
		return fn && fn(this);
	}

	void QAction::setIcon(const QIcon& icon)
	{
		static const auto fn = detail::widgets<void (*)(void*, const void*)>("?setIcon@QAction@@QEAAXAEBVQIcon@@@Z");
		if (fn)
			fn(this, icon.data());
	}

	void* QAction::activate_address()
	{
		return detail::widgets_export("?activate@QAction@@QEAAXW4ActionEvent@1@@Z");
	}
}
