#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace rml::dumper::tests
{
	class ZipBuilder
	{
	public:
		void add_stored(std::string name, std::span<const std::byte> data);
		void add_deflated(std::string name, std::span<const std::byte> data);

		[[nodiscard]] std::vector<std::byte> build() const;
		[[nodiscard]] std::filesystem::path write_to_temporary_file() const;

	private:
		struct Entry
		{
			std::string name;
			std::vector<std::byte> stored;
			std::uint64_t uncompressed_size{};
			std::uint16_t method{};
		};

		std::vector<Entry> m_entries;
	};
}
