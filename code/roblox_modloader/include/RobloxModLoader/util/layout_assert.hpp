#pragma once

#include "RobloxModLoader/internal/platform.hpp"

#include <cstddef>

#if defined(RML_WINDOWS)
	#define RML_ASSERT_LAYOUT_SIZE(type, size) \
		static_assert(sizeof(type) == (size), #type " layout size mismatch")

	#define RML_ASSERT_LAYOUT_OFFSET(type, field, offset) \
		static_assert(offsetof(type, field) == (offset), #type "::" #field " layout offset mismatch")
#else
	#define RML_ASSERT_LAYOUT_SIZE(type, size)
	#define RML_ASSERT_LAYOUT_OFFSET(type, field, offset)
#endif

#define RML_LAYOUT_GUARD_BEGIN() \
	static void rml_assert_layout() noexcept \
	{

#define RML_LAYOUT_GUARD_END() \
	}
