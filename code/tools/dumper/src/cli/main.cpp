#include "cli/runner.hpp"

#include <cstdio>
#include <exception>
#include <spdlog/spdlog.h>

int main(const int argc, char** argv)
{
	try
	{
		std::vector<std::string_view> arguments(argv + 1, argv + argc);

		const auto options = rml::dumper::cli::Options::parse(arguments);
		if (!options)
		{
			std::fputs(options.error().message().c_str(), stderr);
			std::fputc('\n', stderr);
			std::fputs(rml::dumper::cli::Options::usage().c_str(), stderr);
			return static_cast<int>(options.error().code());
		}

		if (options->help)
		{
			std::fputs(rml::dumper::cli::Options::usage().c_str(), stdout);
			return 0;
		}

		spdlog::set_level(options->verbose ? spdlog::level::debug : spdlog::level::info);
		spdlog::set_pattern("%^%l%$ %v");

		if (const auto result = rml::dumper::cli::Runner().run(*options); !result)
		{
			std::fputs(result.error().message().c_str(), stderr);
			std::fputc('\n', stderr);
			return static_cast<int>(result.error().code());
		}

		return 0;
	}
	catch (const std::exception& exception)
	{
		std::fputs(exception.what(), stderr);
		std::fputc('\n', stderr);
		return 1;
	}
}
