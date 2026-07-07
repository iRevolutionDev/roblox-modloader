#pragma once

#include "RobloxModLoader/qt/qt_value_type.hpp"
#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class RML_EXPORT QRect : public detail::QtValueType<16>
	{
	public:
		QRect() = default;
		QRect(int x, int y, int width, int height);
	};
}
