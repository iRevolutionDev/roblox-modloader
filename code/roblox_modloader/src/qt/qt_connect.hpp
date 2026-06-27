#pragma once

#include <functional>

namespace rml::qt::detail
{
	bool connect_function(const void* sender, void* signal_addr, const void* sender_meta, std::function<void(void**)> slot);
}
