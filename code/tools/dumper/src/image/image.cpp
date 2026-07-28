#include "rml/dumper/image/image.hpp"

#include "image/macho_parser.hpp"
#include "image/pe_parser.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <memory>

namespace rml::dumper::image
{
	Image::Image(const ImageFormat format, const Architecture architecture, const Va preferred_base,
	             std::vector<std::byte> memory, std::vector<Section> sections, index::FunctionIndex functions) :
	    m_format(format),
	    m_architecture(architecture),
	    m_preferred_base(preferred_base),
	    m_memory(std::move(memory)),
	    m_sections(std::move(sections)),
	    m_functions(std::move(functions))
	{
		std::ranges::sort(m_sections, {}, &Section::address);

		for (const auto& section : m_sections)
			if (section.executable)
				m_executable_sections.push_back(section);
	}

	std::span<const std::byte> Image::at(const Rva address, const std::size_t size) const
	{
		if (address >= m_memory.size() || size > m_memory.size() - address)
			return {};

		return std::span(m_memory).subspan(address, size);
	}

	std::span<const std::byte> Image::from(const Rva address) const
	{
		if (address >= m_memory.size())
			return {};

		return std::span(m_memory).subspan(address);
	}

	std::optional<Rva> Image::to_rva(const Va address) const
	{
		if (address < m_preferred_base)
			return std::nullopt;

		const auto offset = address - m_preferred_base;
		if (offset >= m_memory.size())
			return std::nullopt;

		return static_cast<Rva>(offset);
	}

	const Section* Image::section_containing(const Rva address) const
	{
		const auto found = std::ranges::find_if(m_sections, [address](const Section& section) {
			return section.contains(address);
		});

		return found != m_sections.end() ? &*found : nullptr;
	}

	std::expected<Image, Error> ImageLoader::load(const std::filesystem::path& path, const Architecture architecture)
	{
		std::error_code ec;
		if (!std::filesystem::exists(path, ec))
			return std::unexpected(Error::make(ErrorCode::invalid_image, "no such file: {}", path.string()));

		const auto size = std::filesystem::file_size(path, ec);
		if (ec)
			return std::unexpected(Error::make(ErrorCode::invalid_image, "cannot size {}: {}", path.string(),
			                                   ec.message()));

		std::ifstream stream(path, std::ios::binary);
		if (!stream)
			return std::unexpected(Error::make(ErrorCode::invalid_image, "cannot open {}", path.string()));

		std::vector<std::byte> file(static_cast<std::size_t>(size));
		stream.read(reinterpret_cast<char*>(file.data()), static_cast<std::streamsize>(file.size()));
		if (!stream)
			return std::unexpected(Error::make(ErrorCode::invalid_image, "cannot read {}", path.string()));

		return parse(std::move(file), architecture);
	}

	std::expected<Image, Error> ImageLoader::parse(std::vector<std::byte> file, const Architecture architecture)
	{
		std::array<std::unique_ptr<ImageParser>, 2> parsers{std::make_unique<PeParser>(),
		                                                    std::make_unique<MachOParser>()};

		for (const auto& parser : parsers)
			if (parser->matches(file))
				return parser->parse(std::move(file), architecture);

		return std::unexpected(Error::make(ErrorCode::invalid_image, "unrecognised container, first bytes are {:02X}",
		                                   file.empty() ? 0 : static_cast<unsigned>(file[0])));
	}
}
