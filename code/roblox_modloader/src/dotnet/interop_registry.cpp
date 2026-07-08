#include "interop_registry.hpp"

#include "RobloxModLoader/internal/common.hpp"

namespace rml::dotnet
{
	InteropRegistry::InteropRegistry()
	{
		m_table.version = RML_INTEROP_VERSION;
		m_table.size    = sizeof(InteropTable);
	}
} // namespace rml::dotnet