#include "image/pe_parser.hpp"

#include <algorithm>

namespace rml::dumper::image
{
	static constexpr std::uint16_t dos_magic = 0x5A4D;
	static constexpr std::uint32_t pe_signature = 0x00004550;
	static constexpr std::uint16_t machine_x86_64 = 0x8664;
	static constexpr std::uint16_t optional_magic_pe32_plus = 0x20B;
	static constexpr std::uint32_t section_characteristic_executable = 0x20000000;
	static constexpr std::size_t exception_directory_index = 3;
	static constexpr std::size_t section_header_size = 40;
	static constexpr std::uint32_t maximum_image_size = 0x8000'0000;

	bool PeParser::matches(const std::span<const std::byte> file) const
	{
		return file.size() >= 2 && file[0] == std::byte{0x4D} && file[1] == std::byte{0x5A};
	}

	std::expected<PeParser::Headers, Error> PeParser::read_headers(const ByteReader& reader,
	                                                               const Architecture architecture)
	{
		const auto magic = reader.read_le<std::uint16_t>(0x00);
		if (!magic)
			return std::unexpected(magic.error());
		if (*magic != dos_magic)
			return std::unexpected(Error::make(ErrorCode::invalid_image, "not a pe image, dos magic is 0x{:X}", *magic));

		const auto pe_offset = reader.read_le<std::uint32_t>(0x3C);
		if (!pe_offset)
			return std::unexpected(pe_offset.error());

		const auto signature = reader.read_le<std::uint32_t>(*pe_offset);
		if (!signature)
			return std::unexpected(signature.error());
		if (*signature != pe_signature)
			return std::unexpected(
			    Error::make(ErrorCode::invalid_image, "pe signature is 0x{:X} at 0x{:X}", *signature, *pe_offset));

		const std::size_t file_header = *pe_offset + 4;

		const auto machine = reader.read_le<std::uint16_t>(file_header + 0x00);
		if (!machine)
			return std::unexpected(machine.error());
		if (*machine != machine_x86_64)
			return std::unexpected(Error::make(ErrorCode::invalid_image,
			                                   "pe machine 0x{:X} does not provide {}", *machine,
			                                   to_string(architecture)));
		if (architecture != Architecture::x86_64)
			return std::unexpected(Error::make(ErrorCode::invalid_image, "pe images do not provide {}",
			                                   to_string(architecture)));

		const auto section_count = reader.read_le<std::uint16_t>(file_header + 0x02);
		const auto optional_size = reader.read_le<std::uint16_t>(file_header + 0x10);
		if (!section_count || !optional_size)
			return std::unexpected(section_count ? optional_size.error() : section_count.error());

		const std::size_t optional_header = file_header + 20;

		const auto optional_magic = reader.read_le<std::uint16_t>(optional_header + 0x00);
		if (!optional_magic)
			return std::unexpected(optional_magic.error());
		if (*optional_magic != optional_magic_pe32_plus)
			return std::unexpected(Error::make(ErrorCode::invalid_image,
			                                   "optional header magic 0x{:X} is not pe32+", *optional_magic));

		const auto image_base = reader.read_le<std::uint64_t>(optional_header + 0x18);
		const auto image_size = reader.read_le<std::uint32_t>(optional_header + 0x38);
		const auto headers_size = reader.read_le<std::uint32_t>(optional_header + 0x3C);
		const auto directory_count = reader.read_le<std::uint32_t>(optional_header + 0x6C);
		if (!image_base || !image_size || !headers_size || !directory_count)
			return std::unexpected(Error::make(ErrorCode::invalid_image, "optional header is truncated"));

		if (*image_size == 0 || *image_size > maximum_image_size)
			return std::unexpected(
			    Error::make(ErrorCode::invalid_image, "implausible size of image 0x{:X}", *image_size));

		Headers headers;
		headers.section_table_offset = optional_header + *optional_size;
		headers.section_count = *section_count;
		headers.image_base = *image_base;
		headers.image_size = *image_size;
		headers.headers_size = *headers_size;

		if (*directory_count > exception_directory_index)
		{
			const auto entry = optional_header + 0x70 + exception_directory_index * 8;
			const auto address = reader.read_le<std::uint32_t>(entry + 0);
			const auto size = reader.read_le<std::uint32_t>(entry + 4);
			if (address && size)
				headers.exception_directory = {*address, *size};
		}

		return headers;
	}

	std::expected<std::vector<PeParser::RawSection>, Error> PeParser::read_sections(const ByteReader& reader,
	                                                                                const Headers& headers)
	{
		std::vector<RawSection> sections;
		sections.reserve(headers.section_count);

		for (std::uint16_t i = 0; i < headers.section_count; ++i)
		{
			const auto entry = headers.section_table_offset + i * section_header_size;

			const auto name = reader.read_fixed_string(entry + 0x00, 8);
			const auto virtual_size = reader.read_le<std::uint32_t>(entry + 0x08);
			const auto virtual_address = reader.read_le<std::uint32_t>(entry + 0x0C);
			const auto raw_size = reader.read_le<std::uint32_t>(entry + 0x10);
			const auto raw_offset = reader.read_le<std::uint32_t>(entry + 0x14);
			const auto characteristics = reader.read_le<std::uint32_t>(entry + 0x24);

			if (!name || !virtual_size || !virtual_address || !raw_size || !raw_offset || !characteristics)
				return std::unexpected(
				    Error::make(ErrorCode::invalid_image, "section header {} is truncated", i));

			RawSection section;
			section.section.name = *name;
			section.section.address = *virtual_address;
			section.section.size = *virtual_size != 0 ? *virtual_size : *raw_size;
			section.section.executable = (*characteristics & section_characteristic_executable) != 0;
			section.file_offset = *raw_offset;
			section.file_size = *raw_size;

			sections.push_back(std::move(section));
		}

		return sections;
	}

	std::vector<std::byte> PeParser::map_sections(const ByteReader& reader, const Headers& headers,
	                                              const std::span<const RawSection> sections)
	{
		std::vector<std::byte> memory(headers.image_size, std::byte{0});

		if (const auto head = reader.slice(0, std::min<std::size_t>(headers.headers_size, reader.size())))
			std::ranges::copy(*head, memory.begin());

		for (const auto& section : sections)
		{
			const auto copy_size = std::min<std::size_t>(section.file_size, section.section.size);
			if (copy_size == 0 || section.section.address >= memory.size())
				continue;

			const auto available = std::min<std::size_t>(copy_size, memory.size() - section.section.address);
			const auto source = reader.slice(section.file_offset, available);
			if (!source)
				continue;

			std::ranges::copy(*source, memory.begin() + section.section.address);
		}

		return memory;
	}

	Rva PeParser::primary_of(const ByteReader& reader, const Rva begin, const Rva unwind_info)
	{
		constexpr std::uint8_t chain_info_flag = 0x04;
		constexpr std::size_t maximum_chain_depth = 8;

		Rva current = begin;
		Rva info = unwind_info;

		for (std::size_t depth = 0; depth < maximum_chain_depth; ++depth)
		{
			const auto header = reader.read_le<std::uint8_t>(info + 0);
			const auto code_count = reader.read_le<std::uint8_t>(info + 2);
			if (!header || !code_count)
				return current;

			if (((*header >> 3) & chain_info_flag) == 0)
				return current;

			const auto chained = info + 4 + 2 * ((*code_count + 1) & ~1u);
			const auto chained_begin = reader.read_le<std::uint32_t>(chained + 0);
			const auto chained_info = reader.read_le<std::uint32_t>(chained + 8);
			if (!chained_begin || !chained_info || *chained_begin == current)
				return current;

			current = *chained_begin;
			info = *chained_info;
		}

		return current;
	}

	index::FunctionIndex PeParser::read_functions(const std::span<const std::byte> mapped, const Directory& directory,
	                                              const std::span<const Section> sections)
	{
		constexpr std::size_t runtime_function_size = 12;

		if (directory.size < runtime_function_size)
			return {};

		const ByteReader reader(mapped);

		const auto executable = [sections](const Rva address) {
			return std::ranges::any_of(sections, [address](const Section& section) {
				return section.executable && section.contains(address);
			});
		};

		std::vector<Chunk> chunks;
		chunks.reserve(directory.size / runtime_function_size);

		for (std::uint32_t offset = 0; offset + runtime_function_size <= directory.size;
		     offset += runtime_function_size)
		{
			const auto entry = directory.address + offset;
			const auto begin = reader.read_le<std::uint32_t>(entry + 0);
			const auto end = reader.read_le<std::uint32_t>(entry + 4);
			const auto unwind = reader.read_le<std::uint32_t>(entry + 8);
			if (!begin || !end || !unwind)
				break;

			if (*end <= *begin || !executable(*begin))
				continue;

			chunks.push_back({*begin, *end, primary_of(reader, *begin, *unwind)});
		}

		std::ranges::sort(chunks, {}, &Chunk::begin);

		std::vector<index::FunctionBounds> functions;
		functions.reserve(chunks.size());

		for (const auto& chunk : chunks)
		{
			const bool continues = !functions.empty() && functions.back().end == chunk.begin &&
			                       chunk.primary != chunk.begin;

			if (continues)
				functions.back().end = chunk.end;
			else
				functions.push_back({chunk.begin, chunk.end});
		}

		return index::FunctionIndex(std::move(functions));
	}

	std::expected<Image, Error> PeParser::parse(std::vector<std::byte> file, const Architecture architecture) const
	{
		const ByteReader reader(file);

		const auto headers = read_headers(reader, architecture);
		if (!headers)
			return std::unexpected(headers.error());

		const auto raw_sections = read_sections(reader, *headers);
		if (!raw_sections)
			return std::unexpected(raw_sections.error());

		auto memory = map_sections(reader, *headers, *raw_sections);

		std::vector<Section> sections;
		sections.reserve(raw_sections->size());
		for (const auto& raw : *raw_sections)
			sections.push_back(raw.section);

		auto functions = read_functions(memory, headers->exception_directory, sections);

		return Image(ImageFormat::pe, Architecture::x86_64, headers->image_base, std::move(memory),
		             std::move(sections), std::move(functions));
	}
}
