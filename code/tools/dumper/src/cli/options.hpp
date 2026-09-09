#pragma once

#include "rml/dumper/core/error.hpp"

#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace rml::dumper::cli
{
	struct Options
	{
		std::optional<std::filesystem::path> input;
		std::string target{"windows-x64"};
		std::string channel{"LIVE"};
		std::filesystem::path out{"dumper_output"};
		std::vector<std::string> emitters;
		std::optional<std::filesystem::path> cache;
		bool verbose{false};
		bool help{false};

		[[nodiscard]] static std::expected<Options, Error> parse(std::span<const std::string_view> arguments);
		[[nodiscard]] static std::string usage();
	};
}
