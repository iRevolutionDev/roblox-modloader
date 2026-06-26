#include "RobloxModLoader/qt/qdialog.hpp"

#include "RobloxModLoader/qt/qt_module.hpp"

namespace rml::qt
{
	int QDialog::exec() const
	{
		static const auto fn = detail::widgets<int (*)(void*)>("?exec@QDialog@@UEAAHXZ");
		return (fn && m_this) ? fn(m_this) : -1;
	}
}
