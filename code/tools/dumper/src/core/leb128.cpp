#include "rml/dumper/core/leb128.hpp"

namespace rml::dumper
{
	std::expected<std::uint64_t, Error> Leb128::decode(const std::span<const std::byte> data, std::size_t& cursor)
	{
		std::uint64_t value = 0;
		std::uint32_t shift = 0;

		while (cursor < data.size())
		{
			const auto byte = static_cast<std::uint8_t>(data[cursor++]);
			if (shift > 63)
				return std::unexpected(Error::make(ErrorCode::invalid_image, "uleb128 value exceeds 64 bits"));

			value |= static_cast<std::uint64_t>(byte & 0x7F) << shift;
			if ((byte & 0x80) == 0)
				return value;

			shift += 7;
		}

		return std::unexpected(Error::make(ErrorCode::invalid_image, "uleb128 runs past the end of the buffer"));
	}

	void Leb128::encode(std::vector<std::byte>& out, std::uint64_t value)
	{
		do
		{
			auto byte = static_cast<std::uint8_t>(value & 0x7F);
			value >>= 7;
			if (value != 0)
				byte |= 0x80;
			out.push_back(static_cast<std::byte>(byte));
		}
		while (value != 0);
	}
}
