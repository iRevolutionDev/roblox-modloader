#pragma once

#include "rml/dumper/core/error.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <vector>

namespace rml::dumper
{
	class Leb128
	{
	public:
		[[nodiscard]] static std::expected<std::uint64_t, Error> decode(std::span<const std::byte> data,
		                                                                std::size_t& cursor);
		static void encode(std::vector<std::byte>& out, std::uint64_t value);
	};
}
