#pragma once

#include "RobloxModLoader/luau/env/mod_environment.hpp"
#include "RobloxModLoader/luau/modules/module_id.hpp"

namespace rml::luau
{
	enum class ResolveError : std::uint8_t
	{
		MissingAlias,
		EmptyName,
		EscapesRoot,
		NotFound,
		NotAFile,
		IoError,
	};

	struct ResolveFailure
	{
		ResolveError error{ResolveError::NotFound};
		std::string specifier;
		std::vector<std::string> attempted;

		[[nodiscard]] std::string describe() const;
	};

	namespace roots
	{
		[[nodiscard]] std::filesystem::path rml_libraries(const ModEnvironment& env) noexcept;
		[[nodiscard]] std::filesystem::path mod_scripts(const ModEnvironment& env) noexcept;
	}

	struct ResolveRule
	{
		std::string_view prefix;
		std::filesystem::path (*root)(const ModEnvironment&) noexcept;
	};

	inline constexpr std::array kResolveRules{
	    ResolveRule{"@rml/", &roots::rml_libraries},
	    ResolveRule{"@self/", &roots::mod_scripts},
	};

	[[nodiscard]] std::expected<ModuleId, ResolveFailure> resolve_module(
	    std::string_view specifier, const ModEnvironment& env);

	[[nodiscard]] bool is_safe_relative_specifier(std::string_view rest) noexcept;
}
