#pragma once

#include "rml/dumper/core/error.hpp"
#include "rml/dumper/core/types.hpp"
#include "rml/dumper/image/section.hpp"
#include "rml/dumper/index/function_index.hpp"

#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <vector>

namespace rml::dumper::image
{
	class Image
	{
	public:
		Image(ImageFormat format, Architecture architecture, Va preferred_base, std::vector<std::byte> memory,
		      std::vector<Section> sections, index::FunctionIndex functions);

		[[nodiscard]] ImageFormat format() const { return m_format; }
		[[nodiscard]] Architecture architecture() const { return m_architecture; }
		[[nodiscard]] Va preferred_base() const { return m_preferred_base; }

		[[nodiscard]] std::span<const std::byte> memory() const { return m_memory; }
		[[nodiscard]] std::span<const Section> sections() const { return m_sections; }
		[[nodiscard]] std::span<const Section> executable_sections() const { return m_executable_sections; }
		[[nodiscard]] const index::FunctionIndex& functions() const { return m_functions; }

		[[nodiscard]] std::span<const std::byte> at(Rva address, std::size_t size) const;
		[[nodiscard]] std::span<const std::byte> from(Rva address) const;
		[[nodiscard]] std::optional<Rva> to_rva(Va address) const;
		[[nodiscard]] const Section* section_containing(Rva address) const;

	private:
		ImageFormat m_format;
		Architecture m_architecture;
		Va m_preferred_base;
		std::vector<std::byte> m_memory;
		std::vector<Section> m_sections;
		std::vector<Section> m_executable_sections;
		index::FunctionIndex m_functions;
	};

	class ImageLoader
	{
	public:
		[[nodiscard]] static std::expected<Image, Error> load(const std::filesystem::path& path,
		                                                      Architecture architecture);
		[[nodiscard]] static std::expected<Image, Error> parse(std::vector<std::byte> file,
		                                                       Architecture architecture);
	};
}
