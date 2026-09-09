#include "image/macho_parser.hpp"

#include "rml/dumper/core/leb128.hpp"

#include <algorithm>

namespace rml::dumper::image
{
	static constexpr std::uint32_t magic_64 = 0xFEEDFACF;
	static constexpr std::uint32_t fat_magic = 0xCAFEBABE;
	static constexpr std::uint32_t fat_magic_64 = 0xCAFEBABF;
	static constexpr std::uint32_t cpu_type_x86_64 = 0x01000007;
	static constexpr std::uint32_t cpu_type_arm64 = 0x0100000C;
	static constexpr std::uint32_t lc_segment_64 = 0x19;
	static constexpr std::uint32_t lc_function_starts = 0x26;
	static constexpr std::uint32_t protection_execute = 0x04;
	static constexpr std::uint32_t section_attribute_pure_instructions = 0x80000000;
	static constexpr std::uint32_t section_attribute_some_instructions = 0x00000400;
	static constexpr std::size_t segment_command_size = 72;
	static constexpr std::size_t section_command_size = 80;
	static constexpr std::uint64_t maximum_image_size = 0x8000'0000;

	std::uint32_t MachOParser::cpu_type_of(const Architecture architecture)
	{
		return architecture == Architecture::arm64 ? cpu_type_arm64 : cpu_type_x86_64;
	}

	bool MachOParser::matches(const std::span<const std::byte> file) const
	{
		const ByteReader reader(file);

		const auto thin = reader.read_le<std::uint32_t>(0);
		if (thin && *thin == magic_64)
			return true;

		const auto fat = reader.read_be<std::uint32_t>(0);
		return fat && (*fat == fat_magic || *fat == fat_magic_64);
	}

	std::expected<std::span<const std::byte>, Error> MachOParser::select_slice(const ByteReader& reader,
	                                                                           const Architecture architecture)
	{
		const auto thin_magic = reader.read_le<std::uint32_t>(0);
		if (!thin_magic)
			return std::unexpected(thin_magic.error());

		if (*thin_magic == magic_64)
		{
			const auto cpu_type = reader.read_le<std::uint32_t>(0x04);
			if (!cpu_type)
				return std::unexpected(cpu_type.error());
			if (*cpu_type != cpu_type_of(architecture))
				return std::unexpected(Error::make(ErrorCode::invalid_image,
				                                   "mach-o cpu type 0x{:X} does not provide {}", *cpu_type,
				                                   to_string(architecture)));

			return reader.slice(0, reader.size());
		}

		const auto fat_magic_value = reader.read_be<std::uint32_t>(0);
		if (!fat_magic_value)
			return std::unexpected(fat_magic_value.error());

		const bool wide = *fat_magic_value == fat_magic_64;
		const std::size_t entry_size = wide ? 32 : 20;

		const auto count = reader.read_be<std::uint32_t>(0x04);
		if (!count)
			return std::unexpected(count.error());

		for (std::uint32_t i = 0; i < *count; ++i)
		{
			const auto entry = 8 + i * entry_size;

			const auto cpu_type = reader.read_be<std::uint32_t>(entry + 0x00);
			if (!cpu_type)
				return std::unexpected(cpu_type.error());
			if (*cpu_type != cpu_type_of(architecture))
				continue;

			std::uint64_t offset{};
			std::uint64_t size{};
			if (wide)
			{
				const auto wide_offset = reader.read_be<std::uint64_t>(entry + 0x08);
				const auto wide_size = reader.read_be<std::uint64_t>(entry + 0x10);
				if (!wide_offset || !wide_size)
					return std::unexpected(Error::make(ErrorCode::invalid_image, "fat entry {} is truncated", i));
				offset = *wide_offset;
				size = *wide_size;
			}
			else
			{
				const auto narrow_offset = reader.read_be<std::uint32_t>(entry + 0x08);
				const auto narrow_size = reader.read_be<std::uint32_t>(entry + 0x0C);
				if (!narrow_offset || !narrow_size)
					return std::unexpected(Error::make(ErrorCode::invalid_image, "fat entry {} is truncated", i));
				offset = *narrow_offset;
				size = *narrow_size;
			}

			return reader.slice(offset, size);
		}

		return std::unexpected(
		    Error::make(ErrorCode::invalid_image, "fat binary carries no {} slice", to_string(architecture)));
	}

	std::expected<MachOParser::RawSection, Error> MachOParser::read_section(const ByteReader& slice,
	                                                                        const std::size_t offset)
	{
		const auto name = slice.read_fixed_string(offset + 0x00, 16);
		const auto address = slice.read_le<std::uint64_t>(offset + 0x20);
		const auto size = slice.read_le<std::uint64_t>(offset + 0x28);
		const auto flags = slice.read_le<std::uint32_t>(offset + 0x40);

		if (!name || !address || !size || !flags)
			return std::unexpected(Error::make(ErrorCode::invalid_image, "section header is truncated"));

		RawSection section;
		section.name = *name;
		section.address = *address;
		section.size = *size;
		section.executable =
		    (*flags & (section_attribute_pure_instructions | section_attribute_some_instructions)) != 0;

		return section;
	}

	std::expected<MachOParser::Contents, Error> MachOParser::read_load_commands(const ByteReader& slice,
	                                                                            const Architecture architecture)
	{
		const auto cpu_type = slice.read_le<std::uint32_t>(0x04);
		if (!cpu_type)
			return std::unexpected(cpu_type.error());
		if (*cpu_type != cpu_type_of(architecture))
			return std::unexpected(Error::make(ErrorCode::invalid_image,
			                                   "mach-o cpu type 0x{:X} does not provide {}", *cpu_type,
			                                   to_string(architecture)));

		const auto command_count = slice.read_le<std::uint32_t>(0x10);
		if (!command_count)
			return std::unexpected(command_count.error());

		Contents contents;
		std::size_t offset = 32;

		for (std::uint32_t i = 0; i < *command_count; ++i)
		{
			const auto command = slice.read_le<std::uint32_t>(offset + 0x00);
			const auto command_size = slice.read_le<std::uint32_t>(offset + 0x04);
			if (!command || !command_size || *command_size < 8)
				return std::unexpected(Error::make(ErrorCode::invalid_image, "load command {} is truncated", i));

			if (*command == lc_segment_64)
			{
				const auto address = slice.read_le<std::uint64_t>(offset + 0x18);
				const auto size = slice.read_le<std::uint64_t>(offset + 0x20);
				const auto file_offset = slice.read_le<std::uint64_t>(offset + 0x28);
				const auto file_size = slice.read_le<std::uint64_t>(offset + 0x30);
				const auto initial_protection = slice.read_le<std::uint32_t>(offset + 0x3C);
				const auto section_count = slice.read_le<std::uint32_t>(offset + 0x40);

				if (!address || !size || !file_offset || !file_size || !initial_protection || !section_count)
					return std::unexpected(Error::make(ErrorCode::invalid_image, "segment {} is truncated", i));

				if (*file_size != 0)
					contents.segments.push_back({*address, *size, *file_offset, *file_size});

				for (std::uint32_t s = 0; s < *section_count; ++s)
				{
					auto section = read_section(slice, offset + segment_command_size + s * section_command_size);
					if (!section)
						return std::unexpected(section.error());

					if ((*initial_protection & protection_execute) == 0)
						section->executable = false;

					contents.sections.push_back(std::move(*section));
				}
			}
			else if (*command == lc_function_starts)
			{
				const auto data_offset = slice.read_le<std::uint32_t>(offset + 0x08);
				const auto data_size = slice.read_le<std::uint32_t>(offset + 0x0C);
				if (!data_offset || !data_size)
					return std::unexpected(Error::make(ErrorCode::invalid_image, "function starts are truncated"));

				contents.function_starts_offset = *data_offset;
				contents.function_starts_size = *data_size;
			}

			offset += *command_size;
		}

		if (contents.segments.empty())
			return std::unexpected(Error::make(ErrorCode::invalid_image, "mach-o image maps no segment"));

		return contents;
	}

	index::FunctionIndex MachOParser::read_functions(const ByteReader& slice, const Contents& contents,
	                                                 const Va preferred_base,
	                                                 const std::span<const Section> sections)
	{
		if (contents.function_starts_size == 0)
			return {};

		const auto data = slice.slice(contents.function_starts_offset, contents.function_starts_size);
		if (!data)
			return {};

		std::vector<Rva> starts;
		std::size_t cursor = 0;
		Va address = preferred_base;

		while (cursor < data->size())
		{
			const auto delta = Leb128::decode(*data, cursor);
			if (!delta || *delta == 0)
				break;

			address += *delta;
			if (address < preferred_base)
				break;

			starts.push_back(static_cast<Rva>(address - preferred_base));
		}

		std::ranges::sort(starts);

		const auto section_end = [sections](const Rva start) -> Rva {
			const auto found = std::ranges::find_if(sections, [start](const Section& section) {
				return section.contains(start);
			});
			return found != sections.end() ? found->end() : start;
		};

		std::vector<index::FunctionBounds> functions;
		functions.reserve(starts.size());

		for (std::size_t i = 0; i < starts.size(); ++i)
		{
			const auto end = i + 1 < starts.size() ? starts[i + 1] : section_end(starts[i]);
			if (end > starts[i])
				functions.push_back({starts[i], end});
		}

		return index::FunctionIndex(std::move(functions));
	}

	std::expected<Image, Error> MachOParser::parse(std::vector<std::byte> file, const Architecture architecture) const
	{
		const ByteReader reader(file);

		const auto slice_bytes = select_slice(reader, architecture);
		if (!slice_bytes)
			return std::unexpected(slice_bytes.error());

		const ByteReader slice(*slice_bytes);

		const auto contents = read_load_commands(slice, architecture);
		if (!contents)
			return std::unexpected(contents.error());

		const auto lowest = std::ranges::min_element(contents->segments, {}, &Segment::address);
		const Va preferred_base = lowest->address;

		std::uint64_t highest = 0;
		for (const auto& segment : contents->segments)
			highest = std::max(highest, segment.address + segment.size);

		const auto image_size = highest - preferred_base;
		if (image_size == 0 || image_size > maximum_image_size)
			return std::unexpected(
			    Error::make(ErrorCode::invalid_image, "implausible mach-o image size 0x{:X}", image_size));

		std::vector<std::byte> memory(image_size, std::byte{0});

		for (const auto& segment : contents->segments)
		{
			const auto destination = segment.address - preferred_base;
			const auto available = std::min<std::uint64_t>(segment.file_size, memory.size() - destination);

			const auto source = slice.slice(segment.file_offset, available);
			if (!source)
				continue;

			std::ranges::copy(*source, memory.begin() + static_cast<std::ptrdiff_t>(destination));
		}

		std::vector<Section> sections;
		sections.reserve(contents->sections.size());
		for (const auto& raw : contents->sections)
		{
			if (raw.address < preferred_base)
				continue;

			sections.push_back({raw.name, static_cast<Rva>(raw.address - preferred_base),
			                    static_cast<std::uint32_t>(raw.size), raw.executable});
		}

		auto functions = read_functions(slice, *contents, preferred_base, sections);

		return Image(ImageFormat::mach_o, architecture, preferred_base, std::move(memory), std::move(sections),
		             std::move(functions));
	}
}
