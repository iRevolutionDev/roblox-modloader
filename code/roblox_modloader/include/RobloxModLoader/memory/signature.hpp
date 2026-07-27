#pragma once

#include "RobloxModLoader/util/compile_time_helpers.hpp"

namespace rml::memory
{
	struct signature
	{
		cstxpr_capped_str<64> m_name;
		cstxpr_capped_str<768> m_ida;
		void (*m_on_signature_found)(memory::handle ptr);
	};
}
