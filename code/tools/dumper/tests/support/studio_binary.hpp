#pragma once

#include "rml/dumper/core/types.hpp"

#include <filesystem>
#include <optional>

namespace rml::dumper::tests
{
	class StudioBinary
	{
	public:
		[[nodiscard]] static std::optional<std::filesystem::path> windows();
		[[nodiscard]] static std::optional<std::filesystem::path> macos(Architecture architecture);

	private:
		[[nodiscard]] static std::optional<std::filesystem::path> from_environment(const char* variable);
	};
}
