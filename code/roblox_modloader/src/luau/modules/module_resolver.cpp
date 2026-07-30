#include "RobloxModLoader/luau/modules/module_resolver.hpp"

RML_LOG_SCOPE("Modules");

namespace rml::luau
{
	namespace roots
	{
		std::filesystem::path rml_libraries(const ModEnvironment& env) noexcept
		{
			return env.rml_libraries;
		}

		std::filesystem::path mod_scripts(const ModEnvironment& env) noexcept
		{
			return env.mod_scripts();
		}
	}

	static std::string join_attempts(const std::vector<std::string>& attempted)
	{
		std::string joined;
		for (const auto& path : attempted)
		{
			if (!joined.empty())
			{
				joined += ", ";
			}
			joined += path;
		}
		return joined.empty() ? std::string{"nothing"} : joined;
	}

	static bool is_inside(const std::filesystem::path& root, const std::filesystem::path& candidate)
	{
		std::error_code ec;
		const auto relative = std::filesystem::relative(candidate, root, ec);
		if (ec || relative.empty())
		{
			return false;
		}

		const auto first = relative.begin();
		return first != relative.end() && *first != "..";
	}

	std::string ResolveFailure::describe() const
	{
		switch (error)
		{
			case ResolveError::MissingAlias:
				return std::format(
				    "module '{}' has no alias: require needs an explicit '@rml/' or '@self/' prefix", specifier);
			case ResolveError::EmptyName:
				return std::format("module '{}' names nothing after its alias", specifier);
			case ResolveError::EscapesRoot:
				return std::format(
				    "module '{}' escapes its root: '..', absolute paths and drive letters are not allowed", specifier);
			case ResolveError::NotAFile:
				return std::format("module '{}' is not a regular file (tried: {})", specifier, join_attempts(attempted));
			case ResolveError::IoError:
				return std::format("module '{}' could not be resolved: its root directory is unavailable", specifier);
			case ResolveError::NotFound:
				break;
		}

		return std::format("module '{}' not found (tried: {})", specifier, join_attempts(attempted));
	}

	bool is_safe_relative_specifier(const std::string_view rest) noexcept
	{
		if (rest.empty())
		{
			return false;
		}

		if (rest.find('\0') != std::string_view::npos || rest.find(':') != std::string_view::npos)
		{
			return false;
		}

		if (rest.front() == '/' || rest.front() == '\\')
		{
			return false;
		}

		std::size_t start = 0;
		while (true)
		{
			std::size_t end = start;
			while (end < rest.size() && rest[end] != '/' && rest[end] != '\\')
			{
				++end;
			}

			if (rest.substr(start, end - start) == "..")
			{
				return false;
			}

			if (end >= rest.size())
			{
				return true;
			}

			start = end + 1;
		}
	}

	std::expected<ModuleId, ResolveFailure> resolve_module(const std::string_view specifier, const ModEnvironment& env)
	{
		ResolveFailure failure{.error = ResolveError::MissingAlias, .specifier = std::string{specifier}};

		const ResolveRule* rule = nullptr;
		for (const auto& candidate : kResolveRules)
		{
			if (specifier.starts_with(candidate.prefix))
			{
				rule = &candidate;
				break;
			}
		}

		if (rule == nullptr)
		{
			return std::unexpected(std::move(failure));
		}

		const auto rest = specifier.substr(rule->prefix.size());
		if (rest.empty())
		{
			failure.error = ResolveError::EmptyName;
			return std::unexpected(std::move(failure));
		}

		if (!is_safe_relative_specifier(rest))
		{
			failure.error = ResolveError::EscapesRoot;
			return std::unexpected(std::move(failure));
		}

		const auto root = rule->root(env);
		if (root.empty())
		{
			failure.error = ResolveError::IoError;
			return std::unexpected(std::move(failure));
		}

		std::error_code ec;
		const auto canonical_root = std::filesystem::weakly_canonical(root, ec);
		if (ec)
		{
			failure.error = ResolveError::IoError;
			return std::unexpected(std::move(failure));
		}

		bool saw_non_file = false;

		for (const std::string_view extension : {".luau", ".lua"})
		{
			const auto candidate = root / (std::string{rest} + std::string{extension});
			failure.attempted.push_back(candidate.generic_string());

			ec.clear();
			const auto status = std::filesystem::status(candidate, ec);
			if (ec || !std::filesystem::exists(status))
			{
				continue;
			}

			if (!std::filesystem::is_regular_file(status))
			{
				saw_non_file = true;
				continue;
			}

			ec.clear();
			const auto resolved = std::filesystem::weakly_canonical(candidate, ec);
			if (ec)
			{
				failure.error = ResolveError::IoError;
				return std::unexpected(std::move(failure));
			}

			if (!is_inside(canonical_root, resolved))
			{
				RML_WARN("Rejected module '{}': '{}' resolves outside '{}'", specifier, resolved.generic_string(),
				         canonical_root.generic_string());
				failure.error = ResolveError::EscapesRoot;
				return std::unexpected(std::move(failure));
			}

			return ModuleId{resolved.generic_string()};
		}

		failure.error = saw_non_file ? ResolveError::NotAFile : ResolveError::NotFound;
		return std::unexpected(std::move(failure));
	}
}
