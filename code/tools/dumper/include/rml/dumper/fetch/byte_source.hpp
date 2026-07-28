#pragma once

#include "rml/dumper/core/error.hpp"

#include <cstddef>
#include <expected>
#include <filesystem>
#include <vector>

namespace rml::dumper::fetch
{
	class ByteSource
	{
	public:
		virtual ~ByteSource() = default;

		[[nodiscard]] virtual std::uint64_t size() const = 0;
		[[nodiscard]] virtual std::expected<std::vector<std::byte>, Error> read(std::uint64_t offset,
		                                                                       std::uint64_t length) = 0;
	};

	class FileSource final : public ByteSource
	{
	public:
		explicit FileSource(std::filesystem::path path);

		[[nodiscard]] std::uint64_t size() const override { return m_size; }
		[[nodiscard]] std::expected<std::vector<std::byte>, Error> read(std::uint64_t offset,
		                                                               std::uint64_t length) override;

		[[nodiscard]] std::uint64_t bytes_read() const { return m_bytes_read; }

	private:
		std::filesystem::path m_path;
		std::uint64_t m_size{};
		std::uint64_t m_bytes_read{};
	};
}
