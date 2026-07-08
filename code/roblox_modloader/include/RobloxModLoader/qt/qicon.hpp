#pragma once

#include "RobloxModLoader/qt/qt_value_type.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QString;

	class RML_EXPORT QIcon : public detail::QtValueType<sizeof(void*)>
	{
	public:
		explicit QIcon(const QString& path);
		~QIcon();

		QIcon(const QIcon&) = delete;
		QIcon& operator=(const QIcon&) = delete;

	private:
		RML_LAYOUT_GUARD_BEGIN()
			RML_ASSERT_LAYOUT_SIZE(decltype(m_storage), STORAGE_SIZE);
		RML_LAYOUT_GUARD_END()
	};
}
