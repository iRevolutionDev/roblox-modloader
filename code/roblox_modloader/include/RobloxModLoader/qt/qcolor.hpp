#pragma once

#include "RobloxModLoader/qt/qt_value_type.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class RML_EXPORT QColor : public detail::QtValueType<24>
	{
	public:
		QColor() = default;
		QColor(int red, int green, int blue, int alpha = 255);
	};
}
