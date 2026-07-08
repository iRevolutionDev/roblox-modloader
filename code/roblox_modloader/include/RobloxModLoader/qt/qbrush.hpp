#pragma once

#include "RobloxModLoader/qt/qt_value_type.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class QPixmap;

	class RML_EXPORT QBrush : public detail::QtValueType<16>
	{
	public:
		explicit QBrush(const QPixmap& texture);
		~QBrush();

		QBrush(const QBrush&) = delete;
		QBrush& operator=(const QBrush&) = delete;
	};
}
