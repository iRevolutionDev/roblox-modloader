#pragma once

#include "RobloxModLoader/luau/env/mod_environment.hpp"
#include "RobloxModLoader/luau/modules/module_id.hpp"

namespace rml::luau
{
	enum class ResolveError : std::uint8_t
	{
		NoPrefix,
		UnknownAlias,
		EmptyName,
		NoRequirer,
		EscapesRoot,
		NotFound,
		NotAFile,
		IoError,
	};

	struct ResolveFailure
	{
		ResolveError error{ResolveError::NotFound};
		std::string specifier;
		std::string requirer;
		std::string alias;
		std::vector<std::string> attempted;

		[[nodiscard]] std::string describe() const;
	};

	struct ResolvedModule
	{
		ModuleId id;
		std::string logical;
	};

	namespace roots
	{
		[[nodiscard]] std::filesystem::path rml_libraries(const ModEnvironment& env) noexcept;
		[[nodiscard]] std::filesystem::path mod_scripts(const ModEnvironment& env) noexcept;
	}

	struct ResolveRule
	{
		std::string_view alias;
		std::filesystem::path (*root)(const ModEnvironment&) noexcept;
	};

	inline constexpr std::array kResolveRules{
	    ResolveRule{"rml", &roots::rml_libraries},
	    ResolveRule{"self", &roots::mod_scripts},
	};

	[[nodiscard]] std::expected<ResolvedModule, ResolveFailure> resolve_module(
	    std::string_view specifier, const ModEnvironment& env, std::string_view requirer = {});

	[[nodiscard]] std::string logical_name_for(const std::filesystem::path& file, const ModEnvironment& env);

	[[nodiscard]] bool is_logical_module_path(std::string_view text) noexcept;
}
