#pragma once
#include "RobloxModLoader/internal/common.hpp"

namespace rml
{
	class IModLoader
	{
	public:
		virtual ~IModLoader() = default;

		[[nodiscard]] virtual std::expected<void, std::string> load(const std::filesystem::path& path)   = 0;
		[[nodiscard]] virtual std::expected<void, std::string> unload(const std::filesystem::path& path) = 0;
		[[nodiscard]] virtual std::expected<void, std::string> reload(const std::filesystem::path& path) = 0;

		[[nodiscard]] virtual void unload_all() = 0;

		[[nodiscard]] virtual std::vector<std::filesystem::path> extensions() const = 0;
	};
}