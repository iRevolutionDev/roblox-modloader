#pragma once

#include "image/image_parser.hpp"

#include "rml/dumper/core/byte_reader.hpp"

namespace rml::dumper::image
{
	class PeParser final : public ImageParser
	{
	public:
		[[nodiscard]] ImageFormat format() const override { return ImageFormat::pe; }
		[[nodiscard]] bool matches(std::span<const std::byte> file) const override;
		[[nodiscard]] std::expected<Image, Error> parse(std::vector<std::byte> file,
		                                                Architecture architecture) const override;

	private:
		struct Directory
		{
			Rva address{};
			std::uint32_t size{};
		};

		struct Headers
		{
			std::size_t section_table_offset{};
			std::uint16_t section_count{};
			Va image_base{};
			std::uint32_t image_size{};
			std::uint32_t headers_size{};
			Directory exception_directory;
		};

		struct RawSection
		{
			Section section;
			std::uint32_t file_offset{};
			std::uint32_t file_size{};
		};

		[[nodiscard]] static std::expected<Headers, Error> read_headers(const ByteReader& reader,
		                                                                Architecture architecture);
		[[nodiscard]] static std::expected<std::vector<RawSection>, Error> read_sections(const ByteReader& reader,
		                                                                                 const Headers& headers);
		[[nodiscard]] static std::vector<std::byte> map_sections(const ByteReader& reader, const Headers& headers,
		                                                         std::span<const RawSection> sections);
		[[nodiscard]] static index::FunctionIndex read_functions(std::span<const std::byte> mapped,
		                                                         const Directory& directory,
		                                                         std::span<const Section> sections);
	};
}
