#include "support/studio_binary.hpp"

#include <cstdlib>

namespace rml::dumper::tests
{
	std::optional<std::filesystem::path> StudioBinary::from_environment(const char* variable)
	{
		const char* value = std::getenv(variable);
		if (value == nullptr || *value == '\0')
			return std::nullopt;

		std::filesystem::path path(value);
		std::error_code ec;
		if (!std::filesystem::exists(path, ec))
			return std::nullopt;

		return path;
	}

	std::optional<std::filesystem::path> StudioBinary::windows()
	{
		return from_environment("RML_TEST_STUDIO");
	}

	std::optional<std::filesystem::path> StudioBinary::macos(const Architecture architecture)
	{
		return from_environment(architecture == Architecture::arm64 ? "RML_TEST_STUDIO_MACOS_ARM64"
		                                                            : "RML_TEST_STUDIO_MACOS_X64");
	}
}
