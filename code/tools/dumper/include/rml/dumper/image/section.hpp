#pragma once

#include "rml/dumper/core/types.hpp"

#include <string>

namespace rml::dumper::image
{
	struct Section
	{
		std::string name;
		Rva address{};
		std::uint32_t size{};
		bool executable{};

		[[nodiscard]] Rva end() const { return address + size; }
		[[nodiscard]] bool contains(const Rva value) const { return value >= address && value < end(); }
	};
}
