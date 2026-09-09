#include "support/zip_builder.hpp"

#include "support/temporary_file.hpp"

#include <algorithm>
#include <cstring>
#include <zlib.h>

namespace rml::dumper::tests
{
	template<typename T>
	static void append(std::vector<std::byte>& buffer, const T value)
	{
		const auto offset = buffer.size();
		buffer.resize(offset + sizeof(T));
		std::memcpy(buffer.data() + offset, &value, sizeof(T));
	}

	void ZipBuilder::add_stored(std::string name, const std::span<const std::byte> data)
	{
		m_entries.push_back({std::move(name), {data.begin(), data.end()}, data.size(), 0});
	}

	void ZipBuilder::add_deflated(std::string name, const std::span<const std::byte> data)
	{
		std::vector<std::byte> compressed(data.size() + data.size() / 2 + 64);

		z_stream stream{};
		deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);

		stream.next_in = reinterpret_cast<Bytef*>(const_cast<std::byte*>(data.data()));
		stream.avail_in = static_cast<uInt>(data.size());
		stream.next_out = reinterpret_cast<Bytef*>(compressed.data());
		stream.avail_out = static_cast<uInt>(compressed.size());

		deflate(&stream, Z_FINISH);
		compressed.resize(stream.total_out);
		deflateEnd(&stream);

		m_entries.push_back({std::move(name), std::move(compressed), data.size(), 8});
	}

	std::vector<std::byte> ZipBuilder::build() const
	{
		std::vector<std::byte> file;
		std::vector<std::uint32_t> offsets;

		for (const auto& entry : m_entries)
		{
			offsets.push_back(static_cast<std::uint32_t>(file.size()));

			append<std::uint32_t>(file, 0x04034B50);
			append<std::uint16_t>(file, 20);
			append<std::uint16_t>(file, 0);
			append<std::uint16_t>(file, entry.method);
			append<std::uint16_t>(file, 0);
			append<std::uint16_t>(file, 0);
			append<std::uint32_t>(file, 0);
			append<std::uint32_t>(file, static_cast<std::uint32_t>(entry.stored.size()));
			append<std::uint32_t>(file, static_cast<std::uint32_t>(entry.uncompressed_size));
			append<std::uint16_t>(file, static_cast<std::uint16_t>(entry.name.size()));
			append<std::uint16_t>(file, 0);

			for (const auto character : entry.name)
				file.push_back(static_cast<std::byte>(character));

			file.insert(file.end(), entry.stored.begin(), entry.stored.end());
		}

		const auto directory_offset = static_cast<std::uint32_t>(file.size());

		for (std::size_t i = 0; i < m_entries.size(); ++i)
		{
			const auto& entry = m_entries[i];

			append<std::uint32_t>(file, 0x02014B50);
			append<std::uint16_t>(file, 20);
			append<std::uint16_t>(file, 20);
			append<std::uint16_t>(file, 0);
			append<std::uint16_t>(file, entry.method);
			append<std::uint16_t>(file, 0);
			append<std::uint16_t>(file, 0);
			append<std::uint32_t>(file, 0);
			append<std::uint32_t>(file, static_cast<std::uint32_t>(entry.stored.size()));
			append<std::uint32_t>(file, static_cast<std::uint32_t>(entry.uncompressed_size));
			append<std::uint16_t>(file, static_cast<std::uint16_t>(entry.name.size()));
			append<std::uint16_t>(file, 0);
			append<std::uint16_t>(file, 0);
			append<std::uint16_t>(file, 0);
			append<std::uint16_t>(file, 0);
			append<std::uint32_t>(file, 0);
			append<std::uint32_t>(file, offsets[i]);

			for (const auto character : entry.name)
				file.push_back(static_cast<std::byte>(character));
		}

		const auto directory_size = static_cast<std::uint32_t>(file.size()) - directory_offset;

		append<std::uint32_t>(file, 0x06054B50);
		append<std::uint16_t>(file, 0);
		append<std::uint16_t>(file, 0);
		append<std::uint16_t>(file, static_cast<std::uint16_t>(m_entries.size()));
		append<std::uint16_t>(file, static_cast<std::uint16_t>(m_entries.size()));
		append<std::uint32_t>(file, directory_size);
		append<std::uint32_t>(file, directory_offset);
		append<std::uint16_t>(file, 0);

		return file;
	}

	std::filesystem::path ZipBuilder::write_to_temporary_file() const
	{
		return TemporaryFile::write(build());
	}
}
