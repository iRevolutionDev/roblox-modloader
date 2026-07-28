#pragma once

#include "rml/dumper/fetch/byte_source.hpp"

#include <string>
#include <string_view>

namespace rml::dumper::fetch
{
	struct ZipEntry
	{
		std::string name;
		std::uint64_t compressed_size{};
		std::uint64_t uncompressed_size{};
		std::uint64_t local_header_offset{};
		std::uint16_t method{};
	};

	class ZipReader
	{
	public:
		[[nodiscard]] static std::expected<ZipReader, Error> open(ByteSource& source);

		[[nodiscard]] const std::vector<ZipEntry>& entries() const { return m_entries; }
		[[nodiscard]] const ZipEntry* find(std::string_view suffix) const;
		[[nodiscard]] std::expected<std::vector<std::byte>, Error> extract(const ZipEntry& entry) const;

	private:
		explicit ZipReader(ByteSource& source) :
		    m_source(&source)
		{
		}

		[[nodiscard]] static std::expected<std::uint64_t, Error> find_directory(ByteSource& source,
		                                                                        std::uint64_t& directory_size);
		[[nodiscard]] std::expected<void, Error> read_directory(std::uint64_t offset, std::uint64_t size);

		ByteSource* m_source;
		std::vector<ZipEntry> m_entries;
	};
}
