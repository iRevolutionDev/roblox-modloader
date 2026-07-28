#pragma once

#include "rml/dumper/core/types.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace rml::dumper::tests
{
	class MachOBuilder
	{
	public:
		explicit MachOBuilder(Architecture architecture) :
		    m_architecture(architecture)
		{
		}

		void add_segment(std::string name, Va address, std::span<const std::byte> data, bool executable = true);
		void add_empty_segment(std::string name, Va address, std::uint64_t size);
		void set_function_starts(std::span<const Rva> starts);

		[[nodiscard]] std::vector<std::byte> build() const;
		[[nodiscard]] std::filesystem::path write_to_temporary_file() const;

		[[nodiscard]] static std::filesystem::path write_fat(std::span<const std::vector<std::byte>> slices);

	private:
		struct Segment
		{
			std::string name;
			Va address{};
			std::uint64_t size{};
			std::vector<std::byte> data;
			bool executable{};
			bool mapped{};
		};

		Architecture m_architecture;
		std::vector<Segment> m_segments;
		std::optional<std::vector<std::byte>> m_function_starts;
	};
}
