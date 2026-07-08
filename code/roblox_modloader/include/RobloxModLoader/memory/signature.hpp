#pragma once

namespace rml::memory
{
	struct signature
	{
		const char* m_name;
		const char* m_ida;
		void (*m_on_signature_found)(memory::handle ptr);
	};
}
