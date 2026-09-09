#pragma once

#include <cstddef>
#include <filesystem>
#include <span>

namespace rml::dumper::tests
{
	class TemporaryFile
	{
	public:
		[[nodiscard]] static std::filesystem::path write(std::span<const std::byte> bytes);
		[[nodiscard]] static std::filesystem::path directory();
	};
}
