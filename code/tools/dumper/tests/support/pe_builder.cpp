#include "support/pe_builder.hpp"

#include "support/temporary_file.hpp"

#include <algorithm>
#include <cstring>

namespace rml::dumper::tests
{
	static std::uint32_t align_up(const std::uint32_t value, const std::uint32_t alignment)
	{
		return (value + alignment - 1) / alignment * alignment;
	}

	template<typename T>
	static void put(std::vector<std::byte>& buffer, const std::size_t offset, const T value)
	{
		std::memcpy(buffer.data() + offset, &value, sizeof(T));
	}

	void PeBuilder::set_directory(const std::size_t index, const Rva address, const std::uint32_t size)
	{
		m_directories[index] = {address, size};
	}

	void PeBuilder::add_section(std::string name, const Rva address, const std::span<const std::byte> data)
	{
		add(std::move(name), address, data, true);
	}

	void PeBuilder::add_data_section(std::string name, const Rva address, const std::span<const std::byte> data)
	{
		add(std::move(name), address, data, false);
	}

	void PeBuilder::add(std::string name, const Rva address, const std::span<const std::byte> data,
	                    const bool executable)
	{
		m_sections.push_back({std::move(name), address, {data.begin(), data.end()}, executable});
	}

	std::uint32_t PeBuilder::size_of_image() const
	{
		std::uint32_t highest = section_alignment;
		for (const auto& section : m_sections)
			highest = std::max(highest, align_up(section.address + static_cast<std::uint32_t>(section.data.size()),
			                                     section_alignment));
		return highest;
	}

	std::vector<std::byte> PeBuilder::build() const
	{
		constexpr std::size_t pe_signature_offset = 0x40;
		constexpr std::size_t file_header_offset = pe_signature_offset + 4;
		constexpr std::size_t optional_header_offset = file_header_offset + 20;
		constexpr std::uint16_t optional_header_size = 0x70 + directory_count * 8;
		const std::size_t section_table_offset = optional_header_offset + optional_header_size;

		const auto headers_size = align_up(
		    static_cast<std::uint32_t>(section_table_offset + m_sections.size() * 40), file_alignment);

		std::vector<std::uint32_t> raw_offsets;
		auto cursor = headers_size;
		for (const auto& section : m_sections)
		{
			raw_offsets.push_back(cursor);
			cursor += align_up(static_cast<std::uint32_t>(section.data.size()), file_alignment);
		}

		std::vector<std::byte> file(cursor, std::byte{0});

		put<std::uint16_t>(file, 0x00, 0x5A4D);
		put<std::uint32_t>(file, 0x3C, static_cast<std::uint32_t>(pe_signature_offset));
		put<std::uint32_t>(file, pe_signature_offset, 0x00004550);

		put<std::uint16_t>(file, file_header_offset + 0x00, m_machine);
		put<std::uint16_t>(file, file_header_offset + 0x02, static_cast<std::uint16_t>(m_sections.size()));
		put<std::uint16_t>(file, file_header_offset + 0x10, optional_header_size);
		put<std::uint16_t>(file, file_header_offset + 0x12, 0x0022);

		put<std::uint16_t>(file, optional_header_offset + 0x00, m_optional_magic);
		put<std::uint64_t>(file, optional_header_offset + 0x18, m_preferred_base);
		put<std::uint32_t>(file, optional_header_offset + 0x20, section_alignment);
		put<std::uint32_t>(file, optional_header_offset + 0x24, file_alignment);
		put<std::uint32_t>(file, optional_header_offset + 0x38, size_of_image());
		put<std::uint32_t>(file, optional_header_offset + 0x3C, headers_size);
		put<std::uint32_t>(file, optional_header_offset + 0x6C, directory_count);

		for (std::size_t i = 0; i < directory_count; ++i)
		{
			const auto offset = optional_header_offset + 0x70 + i * 8;
			put<std::uint32_t>(file, offset + 0, m_directories[i].address);
			put<std::uint32_t>(file, offset + 4, m_directories[i].size);
		}

		for (std::size_t i = 0; i < m_sections.size(); ++i)
		{
			const auto& section = m_sections[i];
			const auto offset = section_table_offset + i * 40;
			const auto raw_size = align_up(static_cast<std::uint32_t>(section.data.size()), file_alignment);

			const auto name_length = std::min<std::size_t>(section.name.size(), 8);
			std::memcpy(file.data() + offset, section.name.data(), name_length);

			put<std::uint32_t>(file, offset + 0x08, static_cast<std::uint32_t>(section.data.size()));
			put<std::uint32_t>(file, offset + 0x0C, section.address);
			put<std::uint32_t>(file, offset + 0x10, raw_size);
			put<std::uint32_t>(file, offset + 0x14, raw_offsets[i]);
			put<std::uint32_t>(file, offset + 0x24, section.executable ? 0x60000020u : 0x40000040u);

			std::ranges::copy(section.data, file.begin() + raw_offsets[i]);
		}

		return file;
	}

	std::filesystem::path PeBuilder::write_to_temporary_file() const
	{
		return TemporaryFile::write(build());
	}

	std::vector<std::byte> PeBuilder::runtime_function_table(const std::span<const std::pair<Rva, Rva>> functions)
	{
		std::vector<std::byte> table(functions.size() * 12, std::byte{0});

		for (std::size_t i = 0; i < functions.size(); ++i)
		{
			put<std::uint32_t>(table, i * 12 + 0, functions[i].first);
			put<std::uint32_t>(table, i * 12 + 4, functions[i].second);
		}

		return table;
	}
}
