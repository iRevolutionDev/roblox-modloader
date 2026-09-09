#include "rml/dumper/scan/pattern.hpp"

#include <algorithm>

namespace rml::dumper::scan
{
	std::expected<std::uint8_t, Error> Pattern::nibble(const char character)
	{
		if (character >= '0' && character <= '9')
			return static_cast<std::uint8_t>(character - '0');
		if (character >= 'a' && character <= 'f')
			return static_cast<std::uint8_t>(character - 'a' + 10);
		if (character >= 'A' && character <= 'F')
			return static_cast<std::uint8_t>(character - 'A' + 10);

		return std::unexpected(Error::make(ErrorCode::usage, "'{}' is not a hex digit", character));
	}

	std::expected<Pattern, Error> Pattern::parse(const std::string_view signature)
	{
		Pattern pattern;
		pattern.m_text = signature;

		for (std::size_t i = 0; i < signature.size();)
		{
			const auto character = signature[i];

			if (character == ' ')
			{
				++i;
				continue;
			}

			if (character == '?')
			{
				pattern.m_bytes.push_back(std::byte{0});
				pattern.m_mask.push_back(std::byte{0});
				i += i + 1 < signature.size() && signature[i + 1] == '?' ? 2 : 1;
				continue;
			}

			if (i + 1 >= signature.size() || signature[i + 1] == ' ')
				return std::unexpected(
				    Error::make(ErrorCode::usage, "'{}' has a dangling nibble at offset {}", signature, i));

			const auto high = nibble(character);
			const auto low = nibble(signature[i + 1]);
			if (!high)
				return std::unexpected(high.error());
			if (!low)
				return std::unexpected(low.error());

			pattern.m_bytes.push_back(static_cast<std::byte>(*high << 4 | *low));
			pattern.m_mask.push_back(std::byte{0xFF});
			i += 2;
		}

		if (pattern.m_bytes.empty())
			return std::unexpected(Error::make(ErrorCode::usage, "'{}' has no bytes", signature));

		const auto concrete = std::ranges::find(pattern.m_mask, std::byte{0xFF});
		if (concrete == pattern.m_mask.end())
			return std::unexpected(
			    Error::make(ErrorCode::usage, "'{}' has no concrete byte to anchor the search on", signature));

		pattern.m_first_concrete_index = static_cast<std::size_t>(concrete - pattern.m_mask.begin());

		return pattern;
	}

	bool Pattern::matches_at(const std::span<const std::byte> data, const std::size_t offset) const
	{
		if (offset > data.size() || m_bytes.size() > data.size() - offset)
			return false;

		for (std::size_t i = 0; i < m_bytes.size(); ++i)
			if ((data[offset + i] & m_mask[i]) != m_bytes[i])
				return false;

		return true;
	}
}
