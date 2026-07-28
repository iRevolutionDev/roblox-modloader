#include "rml/dumper/fetch/zip_reader.hpp"

#include "rml/dumper/core/byte_reader.hpp"

#include <algorithm>
#include <zlib.h>

namespace rml::dumper::fetch
{
	static constexpr std::uint32_t end_of_directory = 0x06054B50;
	static constexpr std::uint32_t end_of_directory_64 = 0x06064B50;
	static constexpr std::uint32_t directory_entry = 0x02014B50;
	static constexpr std::uint32_t saturated32 = 0xFFFFFFFF;
	static constexpr std::uint16_t saturated16 = 0xFFFF;
	static constexpr std::uint64_t tail_window = 66 * 1024;

	std::expected<std::uint64_t, Error> ZipReader::find_directory(ByteSource& source, std::uint64_t& directory_size)
	{
		const auto total = source.size();
		if (total < 22)
			return std::unexpected(Error::make(ErrorCode::fetch, "archive is only {} bytes", total));

		const auto window = std::min(total, tail_window);
		const auto tail = source.read(total - window, window);
		if (!tail)
			return std::unexpected(tail.error());

		const ByteReader reader(*tail);

		std::optional<std::size_t> found;
		for (std::size_t i = tail->size(); i-- >= 4;)
		{
			const auto magic = reader.read_le<std::uint32_t>(i);
			if (magic && *magic == end_of_directory)
			{
				found = i;
				break;
			}

			if (i == 0)
				break;
		}

		if (!found)
			return std::unexpected(Error::make(ErrorCode::fetch, "no end of central directory record"));

		const auto entries = reader.read_le<std::uint16_t>(*found + 10);
		const auto size = reader.read_le<std::uint32_t>(*found + 12);
		const auto offset = reader.read_le<std::uint32_t>(*found + 16);
		if (!entries || !size || !offset)
			return std::unexpected(Error::make(ErrorCode::fetch, "truncated end of central directory"));

		if (*offset != saturated32 && *size != saturated32 && *entries != saturated16)
		{
			directory_size = *size;
			return *offset;
		}

		for (std::size_t i = *found; i-- > 0;)
		{
			const auto magic = reader.read_le<std::uint32_t>(i);
			if (!magic || *magic != end_of_directory_64)
				continue;

			const auto wide_size = reader.read_le<std::uint64_t>(i + 40);
			const auto wide_offset = reader.read_le<std::uint64_t>(i + 48);
			if (!wide_size || !wide_offset)
				return std::unexpected(Error::make(ErrorCode::fetch, "truncated zip64 end of central directory"));

			directory_size = *wide_size;
			return *wide_offset;
		}

		return std::unexpected(Error::make(ErrorCode::fetch, "zip64 archive without a zip64 directory record"));
	}

	std::expected<void, Error> ZipReader::read_directory(const std::uint64_t offset, const std::uint64_t size)
	{
		const auto bytes = m_source->read(offset, size);
		if (!bytes)
			return std::unexpected(bytes.error());

		const ByteReader reader(*bytes);
		std::size_t cursor = 0;

		while (cursor + 46 <= bytes->size())
		{
			const auto magic = reader.read_le<std::uint32_t>(cursor);
			if (!magic || *magic != directory_entry)
				break;

			const auto method = reader.read_le<std::uint16_t>(cursor + 10);
			const auto compressed = reader.read_le<std::uint32_t>(cursor + 20);
			const auto uncompressed = reader.read_le<std::uint32_t>(cursor + 24);
			const auto name_length = reader.read_le<std::uint16_t>(cursor + 28);
			const auto extra_length = reader.read_le<std::uint16_t>(cursor + 30);
			const auto comment_length = reader.read_le<std::uint16_t>(cursor + 32);
			const auto local_offset = reader.read_le<std::uint32_t>(cursor + 42);

			if (!method || !compressed || !uncompressed || !name_length || !extra_length || !comment_length ||
			    !local_offset)
				return std::unexpected(Error::make(ErrorCode::fetch, "truncated central directory entry"));

			const auto name = reader.slice(cursor + 46, *name_length);
			if (!name)
				return std::unexpected(name.error());

			ZipEntry entry;
			entry.name.assign(reinterpret_cast<const char*>(name->data()), name->size());
			entry.method = *method;
			entry.compressed_size = *compressed;
			entry.uncompressed_size = *uncompressed;
			entry.local_header_offset = *local_offset;

			if (entry.compressed_size == saturated32 || entry.uncompressed_size == saturated32 ||
			    entry.local_header_offset == saturated32)
			{
				const auto extra = reader.slice(cursor + 46 + *name_length, *extra_length);
				if (!extra)
					return std::unexpected(extra.error());

				const ByteReader fields(*extra);
				std::size_t field = 0;

				while (field + 4 <= extra->size())
				{
					const auto tag = fields.read_le<std::uint16_t>(field);
					const auto length = fields.read_le<std::uint16_t>(field + 2);
					if (!tag || !length)
						break;

					if (*tag == 0x0001)
					{
						std::size_t value = field + 4;

						if (entry.uncompressed_size == saturated32)
							if (const auto wide = fields.read_le<std::uint64_t>(value))
							{
								entry.uncompressed_size = *wide;
								value += 8;
							}

						if (entry.compressed_size == saturated32)
							if (const auto wide = fields.read_le<std::uint64_t>(value))
							{
								entry.compressed_size = *wide;
								value += 8;
							}

						if (entry.local_header_offset == saturated32)
							if (const auto wide = fields.read_le<std::uint64_t>(value))
								entry.local_header_offset = *wide;

						break;
					}

					field += 4 + *length;
				}
			}

			m_entries.push_back(std::move(entry));
			cursor += 46 + *name_length + *extra_length + *comment_length;
		}

		return {};
	}

	std::expected<ZipReader, Error> ZipReader::open(ByteSource& source)
	{
		std::uint64_t directory_size = 0;

		const auto offset = find_directory(source, directory_size);
		if (!offset)
			return std::unexpected(offset.error());

		ZipReader reader(source);
		if (const auto read = reader.read_directory(*offset, directory_size); !read)
			return std::unexpected(read.error());

		return reader;
	}

	const ZipEntry* ZipReader::find(const std::string_view suffix) const
	{
		const auto found = std::ranges::find_if(m_entries, [suffix](const ZipEntry& entry) {
			return entry.name.ends_with(suffix);
		});

		return found != m_entries.end() ? &*found : nullptr;
	}

	std::expected<std::vector<std::byte>, Error> ZipReader::extract(const ZipEntry& entry) const
	{
		const auto header = m_source->read(entry.local_header_offset, 30);
		if (!header)
			return std::unexpected(header.error());

		const ByteReader reader(*header);
		const auto name_length = reader.read_le<std::uint16_t>(26);
		const auto extra_length = reader.read_le<std::uint16_t>(28);
		if (!name_length || !extra_length)
			return std::unexpected(Error::make(ErrorCode::fetch, "truncated local header for {}", entry.name));

		const auto data = m_source->read(entry.local_header_offset + 30 + *name_length + *extra_length,
		                                 entry.compressed_size);
		if (!data)
			return std::unexpected(data.error());

		if (entry.method == 0)
			return *data;

		if (entry.method != 8)
			return std::unexpected(
			    Error::make(ErrorCode::fetch, "{} uses compression method {}", entry.name, entry.method));

		std::vector<std::byte> out(static_cast<std::size_t>(entry.uncompressed_size));

		z_stream stream{};
		if (inflateInit2(&stream, -15) != Z_OK)
			return std::unexpected(Error::make(ErrorCode::fetch, "cannot start inflating {}", entry.name));

		stream.next_in = reinterpret_cast<Bytef*>(const_cast<std::byte*>(data->data()));
		stream.avail_in = static_cast<uInt>(data->size());
		stream.next_out = reinterpret_cast<Bytef*>(out.data());
		stream.avail_out = static_cast<uInt>(out.size());

		const auto status = inflate(&stream, Z_FINISH);
		const auto produced = stream.total_out;
		inflateEnd(&stream);

		if (status != Z_STREAM_END || produced != entry.uncompressed_size)
			return std::unexpected(Error::make(ErrorCode::fetch, "{} inflated to {} of {} bytes", entry.name,
			                                   produced, entry.uncompressed_size));

		return out;
	}
}
