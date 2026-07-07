#pragma once

#include "RobloxModLoader/rml_export.hpp"

namespace rml::qt
{
	class RML_EXPORT QPaintDevice
	{
	protected:
		void* m_layout_probe = nullptr;
	};
}
