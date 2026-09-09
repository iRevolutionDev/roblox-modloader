#include "support/temporary_file.hpp"

#include <atomic>
#include <format>
#include <fstream>

namespace rml::dumper::tests
{
	static std::atomic<std::uint64_t> s_counter{0};

	std::filesystem::path TemporaryFile::directory()
	{
		auto path = std::filesystem::temp_directory_path() / "rml_dumper_tests";
		std::error_code ec;
		std::filesystem::create_directories(path, ec);
		return path;
	}

	std::filesystem::path TemporaryFile::write(const std::span<const std::byte> bytes)
	{
		const auto path = directory() / std::format("fixture_{}.bin", s_counter.fetch_add(1));

		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

		return path;
	}
}
