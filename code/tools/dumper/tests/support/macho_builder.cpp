#include "support/macho_builder.hpp"

#include "rml/dumper/core/byte_reader.hpp"
#include "rml/dumper/core/leb128.hpp"
#include "support/temporary_file.hpp"

#include <algorithm>
#include <bit>
#include <cstring>

namespace rml::dumper::tests
{
	static constexpr std::uint32_t macho_magic_64 = 0xFEEDFACF;
	static constexpr std::uint32_t fat_magic = 0xCAFEBABE;
	static constexpr std::uint32_t cpu_type_x86_64 = 0x01000007;
	static constexpr std::uint32_t cpu_type_arm64 = 0x0100000C;
	static constexpr std::uint32_t lc_segment_64 = 0x19;
	static constexpr std::uint32_t lc_function_starts = 0x26;
	static constexpr std::size_t segment_command_size = 72;
	static constexpr std::size_t section_command_size = 80;

	template<typename T>
	static void put_le(std::vector<std::byte>& buffer, const std::size_t offset, const T value)
	{
		std::memcpy(buffer.data() + offset, &value, sizeof(T));
	}

	template<typename T>
	static void put_be(std::vector<std::byte>& buffer, const std::size_t offset, T value)
	{
		value = std::byteswap(value);
		std::memcpy(buffer.data() + offset, &value, sizeof(T));
	}

	static void put_name(std::vector<std::byte>& buffer, const std::size_t offset, const std::string& name)
	{
		std::memcpy(buffer.data() + offset, name.data(), std::min<std::size_t>(name.size(), 16));
	}

	void MachOBuilder::add_segment(std::string name, const Va address, const std::span<const std::byte> data,
	                               const bool executable)
	{
		m_segments.push_back({std::move(name), address, data.size(), {data.begin(), data.end()}, executable, true});
	}

	void MachOBuilder::add_empty_segment(std::string name, const Va address, const std::uint64_t size)
	{
		m_segments.push_back({std::move(name), address, size, {}, false, false});
	}

	void MachOBuilder::set_function_starts(const std::span<const Rva> starts)
	{
		std::vector<std::byte> encoded;
		Rva previous = 0;
		for (const auto start : starts)
		{
			Leb128::encode(encoded, start - previous);
			previous = start;
		}
		encoded.push_back(std::byte{0});

		m_function_starts = std::move(encoded);
	}

	std::vector<std::byte> MachOBuilder::build() const
	{
		const std::size_t mapped_count = std::ranges::count_if(m_segments, [](const Segment& s) { return s.mapped; });

		std::size_t commands_size = m_segments.size() * segment_command_size + mapped_count * section_command_size;
		if (m_function_starts)
			commands_size += 16;

		std::size_t cursor = 32 + commands_size;

		std::vector<std::size_t> data_offsets;
		for (const auto& segment : m_segments)
		{
			data_offsets.push_back(segment.mapped ? cursor : 0);
			if (segment.mapped)
				cursor += segment.data.size();
		}

		const std::size_t function_starts_offset = cursor;
		if (m_function_starts)
			cursor += m_function_starts->size();

		std::vector<std::byte> file(cursor, std::byte{0});

		put_le<std::uint32_t>(file, 0x00, macho_magic_64);
		put_le<std::uint32_t>(file, 0x04,
		                      m_architecture == Architecture::arm64 ? cpu_type_arm64 : cpu_type_x86_64);
		put_le<std::uint32_t>(file, 0x0C, 2);
		put_le<std::uint32_t>(file, 0x10,
		                      static_cast<std::uint32_t>(m_segments.size() + (m_function_starts ? 1 : 0)));
		put_le<std::uint32_t>(file, 0x14, static_cast<std::uint32_t>(commands_size));

		std::size_t command = 32;
		for (std::size_t i = 0; i < m_segments.size(); ++i)
		{
			const auto& segment = m_segments[i];
			const auto sections = segment.mapped ? 1u : 0u;
			const auto command_size = segment_command_size + sections * section_command_size;

			put_le<std::uint32_t>(file, command + 0x00, lc_segment_64);
			put_le<std::uint32_t>(file, command + 0x04, static_cast<std::uint32_t>(command_size));
			put_name(file, command + 0x08, segment.name);
			put_le<std::uint64_t>(file, command + 0x18, segment.address);
			put_le<std::uint64_t>(file, command + 0x20, segment.size);
			put_le<std::uint64_t>(file, command + 0x28, data_offsets[i]);
			put_le<std::uint64_t>(file, command + 0x30, segment.data.size());
			put_le<std::uint32_t>(file, command + 0x38, segment.executable ? 5 : 3);
			put_le<std::uint32_t>(file, command + 0x3C, segment.executable ? 5 : 3);
			put_le<std::uint32_t>(file, command + 0x40, sections);

			if (segment.mapped)
			{
				const auto section = command + segment_command_size;
				put_name(file, section + 0x00, segment.executable ? "__text" : "__data");
				put_name(file, section + 0x10, segment.name);
				put_le<std::uint64_t>(file, section + 0x20, segment.address);
				put_le<std::uint64_t>(file, section + 0x28, segment.data.size());
				put_le<std::uint32_t>(file, section + 0x30, static_cast<std::uint32_t>(data_offsets[i]));
				put_le<std::uint32_t>(file, section + 0x40, segment.executable ? 0x80000400u : 0u);

				std::ranges::copy(segment.data, file.begin() + static_cast<std::ptrdiff_t>(data_offsets[i]));
			}

			command += command_size;
		}

		if (m_function_starts)
		{
			put_le<std::uint32_t>(file, command + 0x00, lc_function_starts);
			put_le<std::uint32_t>(file, command + 0x04, 16);
			put_le<std::uint32_t>(file, command + 0x08, static_cast<std::uint32_t>(function_starts_offset));
			put_le<std::uint32_t>(file, command + 0x0C, static_cast<std::uint32_t>(m_function_starts->size()));

			std::ranges::copy(*m_function_starts,
			                  file.begin() + static_cast<std::ptrdiff_t>(function_starts_offset));
		}

		return file;
	}

	std::filesystem::path MachOBuilder::write_to_temporary_file() const
	{
		return TemporaryFile::write(build());
	}

	std::filesystem::path MachOBuilder::write_fat(const std::span<const std::vector<std::byte>> slices)
	{
		constexpr std::size_t alignment = 0x4000;

		const std::size_t header_size = 8 + slices.size() * 20;
		std::size_t cursor = (header_size + alignment - 1) / alignment * alignment;

		std::vector<std::size_t> offsets;
		for (const auto& slice : slices)
		{
			offsets.push_back(cursor);
			cursor += (slice.size() + alignment - 1) / alignment * alignment;
		}

		std::vector<std::byte> file(cursor, std::byte{0});

		put_be<std::uint32_t>(file, 0x00, fat_magic);
		put_be<std::uint32_t>(file, 0x04, static_cast<std::uint32_t>(slices.size()));

		for (std::size_t i = 0; i < slices.size(); ++i)
		{
			const ByteReader reader(slices[i]);
			const auto cpu_type = reader.read_le<std::uint32_t>(0x04);

			const auto entry = 8 + i * 20;
			put_be<std::uint32_t>(file, entry + 0x00, cpu_type.value_or(0));
			put_be<std::uint32_t>(file, entry + 0x08, static_cast<std::uint32_t>(offsets[i]));
			put_be<std::uint32_t>(file, entry + 0x0C, static_cast<std::uint32_t>(slices[i].size()));

			std::ranges::copy(slices[i], file.begin() + static_cast<std::ptrdiff_t>(offsets[i]));
		}

		return TemporaryFile::write(file);
	}
}
