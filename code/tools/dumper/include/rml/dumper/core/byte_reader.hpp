#pragma once

#include "rml/dumper/core/error.hpp"

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <expected>
#include <span>
#include <string>

namespace rml::dumper
{
	class ByteReader
	{
	public:
		explicit ByteReader(const std::span<const std::byte> data) :
		    m_data(data)
		{
		}

		[[nodiscard]] std::size_t size() const { return m_data.size(); }

		[[nodiscard]] std::expected<std::span<const std::byte>, Error> slice(const std::size_t offset,
		                                                                     const std::size_t size) const
		{
			if (offset > m_data.size() || size > m_data.size() - offset)
				return std::unexpected(Error::make(ErrorCode::invalid_image,
				                                   "read of {} bytes at 0x{:X} exceeds the {} byte buffer", size,
				                                   offset, m_data.size()));
			return m_data.subspan(offset, size);
		}

		template<std::unsigned_integral T>
		[[nodiscard]] std::expected<T, Error> read_le(const std::size_t offset) const
		{
			const auto bytes = slice(offset, sizeof(T));
			if (!bytes)
				return std::unexpected(bytes.error());

			T value{};
			std::memcpy(&value, bytes->data(), sizeof(T));

			if constexpr (std::endian::native == std::endian::big)
				value = std::byteswap(value);

			return value;
		}

		template<std::unsigned_integral T>
		[[nodiscard]] std::expected<T, Error> read_be(const std::size_t offset) const
		{
			const auto value = read_le<T>(offset);
			if (!value)
				return std::unexpected(value.error());

			if constexpr (sizeof(T) == 1)
				return *value;
			else
				return std::byteswap(*value);
		}

		[[nodiscard]] std::expected<std::string, Error> read_fixed_string(const std::size_t offset,
		                                                                  const std::size_t capacity) const
		{
			const auto bytes = slice(offset, capacity);
			if (!bytes)
				return std::unexpected(bytes.error());

			std::string value;
			value.reserve(capacity);
			for (const auto byte : *bytes)
			{
				if (byte == std::byte{0})
					break;
				value.push_back(static_cast<char>(byte));
			}
			return value;
		}

	private:
		std::span<const std::byte> m_data;
	};
}
