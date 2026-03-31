#pragma once
#include "dotnet_runtime.hpp"
#include "interop_registry.hpp"

#include <filesystem>

namespace rml::dotnet
{
	class ManagedBridge
	{
	public:
		explicit ManagedBridge(DotnetRuntime& runtime, InteropRegistry& registry) :
		    m_runtime(runtime),
		    m_registry(registry)
		{
		}

		[[nodiscard]] std::expected<void, std::string> initialize(const std::filesystem::path& native_host_dll, const std::filesystem::path& mods_root);

		[[nodiscard]] std::expected<void, std::string> load_mod(const std::filesystem::path& path) const;
		[[nodiscard]] std::expected<void, std::string> unload_mod(const std::filesystem::path& path) const;
		[[nodiscard]] std::expected<void, std::string> shutdown() const;

	private:
		using InitFn     = int32_t(__cdecl*)(const char* mods_root, InteropTable* table);
		using LoadModFn  = int32_t(__cdecl*)(const char* assembly_path);
		using UnloadFn   = int32_t(__cdecl*)(const char* assembly_path);
		using ShutdownFn = void(__cdecl*)();

		DotnetRuntime& m_runtime;
		InteropRegistry& m_registry;

		InitFn m_initialize{};
		LoadModFn m_load_mod{};
		UnloadFn m_unload_mod{};
		ShutdownFn m_shutdown{};
	};

} // namespace rml::dotnet