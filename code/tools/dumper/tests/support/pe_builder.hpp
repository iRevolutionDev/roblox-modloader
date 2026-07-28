#pragma once

#include "rml/dumper/core/types.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace rml::dumper::tests
{
	class PeBuilder
	{
	public:
		void set_preferred_base(Va base) { m_preferred_base = base; }
		void set_machine(std::uint16_t machine) { m_machine = machine; }
		void set_optional_magic(std::uint16_t magic) { m_optional_magic = magic; }
		void set_directory(std::size_t index, Rva address, std::uint32_t size);

		void add_section(std::string name, Rva address, std::span<const std::byte> data);
		void add_data_section(std::string name, Rva address, std::span<const std::byte> data);

		[[nodiscard]] std::vector<std::byte> build() const;
		[[nodiscard]] std::filesystem::path write_to_temporary_file() const;

		[[nodiscard]] static std::vector<std::byte> runtime_function_table(
		    std::span<const std::pair<Rva, Rva>> functions);

	private:
		struct Entry
		{
			std::string name;
			Rva address{};
			std::vector<std::byte> data;
			bool executable{};
		};

		struct Directory
		{
			Rva address{};
			std::uint32_t size{};
		};

		static constexpr std::size_t directory_count = 16;
		static constexpr std::uint32_t section_alignment = 0x1000;
		static constexpr std::uint32_t file_alignment = 0x200;

		void add(std::string name, Rva address, std::span<const std::byte> data, bool executable);
		[[nodiscard]] std::uint32_t size_of_image() const;

		Va m_preferred_base{0x140000000};
		std::uint16_t m_machine{0x8664};
		std::uint16_t m_optional_magic{0x20B};
		std::vector<Entry> m_sections;
		std::array<Directory, directory_count> m_directories{};
	};
}
