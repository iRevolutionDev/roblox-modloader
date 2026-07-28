#pragma once

#include "image/image_parser.hpp"

#include "rml/dumper/core/byte_reader.hpp"

namespace rml::dumper::image
{
	class MachOParser final : public ImageParser
	{
	public:
		[[nodiscard]] ImageFormat format() const override { return ImageFormat::mach_o; }
		[[nodiscard]] bool matches(std::span<const std::byte> file) const override;
		[[nodiscard]] std::expected<Image, Error> parse(std::vector<std::byte> file,
		                                                Architecture architecture) const override;

	private:
		struct Segment
		{
			Va address{};
			std::uint64_t size{};
			std::uint64_t file_offset{};
			std::uint64_t file_size{};
		};

		struct RawSection
		{
			std::string name;
			Va address{};
			std::uint64_t size{};
			bool executable{};
		};

		struct Contents
		{
			std::vector<Segment> segments;
			std::vector<RawSection> sections;
			std::uint32_t function_starts_offset{};
			std::uint32_t function_starts_size{};
		};

		[[nodiscard]] static std::expected<std::span<const std::byte>, Error> select_slice(
		    const ByteReader& reader, Architecture architecture);
		[[nodiscard]] static std::expected<Contents, Error> read_load_commands(const ByteReader& slice,
		                                                                       Architecture architecture);
		[[nodiscard]] static std::expected<RawSection, Error> read_section(const ByteReader& slice,
		                                                                   std::size_t offset);
		[[nodiscard]] static index::FunctionIndex read_functions(const ByteReader& slice, const Contents& contents,
		                                                         Va preferred_base,
		                                                         std::span<const Section> sections);
		[[nodiscard]] static std::uint32_t cpu_type_of(Architecture architecture);
	};
}
