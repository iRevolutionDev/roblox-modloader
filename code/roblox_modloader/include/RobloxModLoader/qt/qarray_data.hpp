#pragma once

#include <cstddef>

namespace rml::qt::detail
{
	void release_array_data(void*& d, std::size_t element_size);
	void destroy_qstring(void*& d);
	void destroy_qbytearray(void*& d);
	[[nodiscard]] const char* array_data_begin(const void* d);
}
