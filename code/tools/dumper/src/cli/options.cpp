#include "cli/options.hpp"

#include "rml/dumper/emit/emitter.hpp"
#include "rml/dumper/target/target_profile.hpp"

#include <algorithm>
#include <format>

namespace rml::dumper::cli
{
	static std::vector<std::string> split(const std::string_view text, const char separator)
	{
		std::vector<std::string> parts;
		std::size_t start = 0;

		while (start <= text.size())
		{
			const auto found = text.find(separator, start);
			const auto end = found == std::string_view::npos ? text.size() : found;

			if (end > start)
				parts.emplace_back(text.substr(start, end - start));

			if (found == std::string_view::npos)
				break;

			start = found + 1;
		}

		return parts;
	}

	std::string Options::usage()
	{
		return std::format(
		    "usage: dumper [options]\n"
		    "  --input <file>      analyse a local studio binary instead of fetching one\n"
		    "  --target <name>     one of {}\n"
		    "  --channel <name>    deployment channel to fetch from, default LIVE\n"
		    "  --out <dir>         where to write the generated files, default dumper_output\n"
		    "  --emit <a,b>        which formats to write, default all of {}\n"
		    "  --cache <dir>       keep fetched studio builds here\n"
		    "  --verbose           log every step\n"
		    "  --help              show this text\n",
		    target::TargetRegistry::names(), "json, report, shuffle");
	}

	std::expected<Options, Error> Options::parse(const std::span<const std::string_view> arguments)
	{
		Options options;

		const auto value_of = [&arguments](const std::size_t index,
		                                   const std::string_view flag) -> std::expected<std::string_view, Error> {
			if (index + 1 >= arguments.size())
				return std::unexpected(Error::make(ErrorCode::usage, "{} needs a value", flag));

			return arguments[index + 1];
		};

		for (std::size_t i = 0; i < arguments.size(); ++i)
		{
			const auto argument = arguments[i];

			if (argument == "--help" || argument == "-h")
			{
				options.help = true;
				continue;
			}

			if (argument == "--verbose")
			{
				options.verbose = true;
				continue;
			}

			const auto value = value_of(i, argument);

			if (argument == "--input")
			{
				if (!value)
					return std::unexpected(value.error());
				options.input = std::filesystem::path(*value);
			}
			else if (argument == "--target")
			{
				if (!value)
					return std::unexpected(value.error());
				options.target = *value;
			}
			else if (argument == "--channel")
			{
				if (!value)
					return std::unexpected(value.error());
				options.channel = *value;
			}
			else if (argument == "--out")
			{
				if (!value)
					return std::unexpected(value.error());
				options.out = std::filesystem::path(*value);
			}
			else if (argument == "--cache")
			{
				if (!value)
					return std::unexpected(value.error());
				options.cache = std::filesystem::path(*value);
			}
			else if (argument == "--emit")
			{
				if (!value)
					return std::unexpected(value.error());
				options.emitters = split(*value, ',');
			}
			else
			{
				return std::unexpected(Error::make(ErrorCode::usage, "unknown option {}", argument));
			}

			++i;
		}

		if (target::TargetRegistry::find(options.target) == nullptr)
			return std::unexpected(Error::make(ErrorCode::usage, "unknown target {}, expected one of {}",
			                                   options.target, target::TargetRegistry::names()));

		for (const auto& id : options.emitters)
			if (emit::EmitterRegistry().create(id) == nullptr)
				return std::unexpected(
				    Error::make(ErrorCode::usage, "unknown format {}, expected one of json, report, shuffle", id));

		return options;
	}
}
