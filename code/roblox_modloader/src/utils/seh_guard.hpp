#pragma once

namespace rml::utils
{
	bool guarded_invoke(void (*fn)(void* ctx), void* ctx) noexcept;
}
