#pragma once

#include "rml/dumper/core/error.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

namespace rml::dumper::scan
{
	class Pattern
	{
	public:
		[[nodiscard]] static std::expected<Pattern, Error> parse(std::string_view signature);

		[[nodiscard]] std::size_t size() const { return m_bytes.size(); }
		[[nodiscard]] std::size_t first_concrete_index() const { return m_first_concrete_index; }
		[[nodiscard]] std::byte first_concrete_byte() const { return m_bytes[m_first_concrete_index]; }
		[[nodiscard]] std::string_view text() const { return m_text; }

		[[nodiscard]] bool matches_at(std::span<const std::byte> data, std::size_t offset) const;

	private:
		Pattern() = default;

		[[nodiscard]] static std::expected<std::uint8_t, Error> nibble(char character);

		std::vector<std::byte> m_bytes;
		std::vector<std::byte> m_mask;
		std::size_t m_first_concrete_index{};
		std::string m_text;
	};
}
