#pragma once

#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class RML_EXPORT QMetaObject
	{
	public:
		[[nodiscard]] const char* className() const;
	};
}
